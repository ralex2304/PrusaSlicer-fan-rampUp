#ifndef _GCODE_H
#define _GCODE_H
#include <stdlib.h>
#include <assert.h>
#include "strings.h"
#include "cvector.h"
#include "errors.h"

#define LINE_MAX_LEN 2500

typedef struct {
  double X;
  double Y;
  double Z;
  double E;
  int XYZabs;
  int Eabs;
  double fan_prev;
} Pos;


typedef struct {
  wchar_t* str;
  Pos pos;
  double time;
}  GcodeLine;

void gcode_init_line(GcodeLine* line);
void gcode_parse_line(GcodeLine* line);
unsigned long find_line_by_time(cvector* lines, const double target);
int gcode_split_line(GcodeLine* cur_line, GcodeLine* bef_line, double delta, GcodeLine** out1, GcodeLine** out2);
void remove_GXYZE(wchar_t* dest, wchar_t* src);


void gcode_init_line(GcodeLine* line) {
  line->str = NULL;
  line->pos.X = 0.0;
  line->pos.Y = 0.0;
  line->pos.Z = 0.0;
  line->pos.E = 0.0;
  line->pos.XYZabs = 1;
  line->pos.Eabs = 0;
  line->pos.fan_prev = 0;
  line->time = 0;
}

void gcode_parse_line(GcodeLine* line) {
  if (!(startswith(line->str, L"G") || startswith(line->str, L"M"))) {
    return;
  } else if (startswith(line->str, L"G0 ") || startswith(line->str, L"G1 ")) {
    if (line->pos.XYZabs) {
      get_param_ptr(line->str, L"X", &line->pos.X);
      get_param_ptr(line->str, L"Y", &line->pos.Y);
      get_param_ptr(line->str, L"Z", &line->pos.Z);
    } else {
      double buf = 0.0;
      get_param_ptr(line->str, L"X", &buf);
      line->pos.X += buf;
      buf = 0.0;
      get_param_ptr(line->str, L"Y", &buf);
      line->pos.Y += buf;
      buf = 0.0;
      get_param_ptr(line->str, L"Z", &buf);
      line->pos.Z += buf;
    }
    if (line->pos.Eabs) {
      get_param_ptr(line->str, L"E", &line->pos.E);
    } else {
      double buf = 0.0;
      get_param_ptr(line->str, L"E", &buf);
      line->pos.E += buf;
    }
  } else if (startswith(line->str, L"G90")) {
    line->pos.XYZabs = 1;
  } else if (startswith(line->str, L"G91")) {
    line->pos.XYZabs = 0;
  } else if (startswith(line->str, L"M82")) {
    line->pos.Eabs = 1;
  } else if (startswith(line->str, L"M83")) {
    line->pos.Eabs = 0;
  }
  return;
}

unsigned long find_line_by_time(cvector* lines, const double target) {
  if (target <= 0)
    return 0;
  unsigned long L = 0;
  unsigned long R = lines->size;
  unsigned long M;
  while (R-L > 1) {
    M = (L + R) / 2;
    if (target >= ((GcodeLine*) cvector_get(lines, M))->time)
      L = M;
    else
      R = M;
  }
  return L + 1;
}

int gcode_split_line(GcodeLine* cur_line, GcodeLine* bef_line, double delta, GcodeLine** out1, GcodeLine** out2) {
  double full_time = cur_line->time - bef_line->time;
  if (!(startswith(cur_line->str, L"G0 ") || startswith(cur_line->str, L"G1 "))) {
    if (!(startswith(cur_line->str, L"G10") || startswith(cur_line->str, L"G11")))
      throw_warning(WARN_SPLIT_ERROR, cur_line->str);
    return 0;
  }
  if (*out1 != NULL) free(*out1);
  if (*out2 != NULL) free(*out2);
  wchar_t prefix[3] = L"";
  wcsncpy(prefix, cur_line->str, 2);
  wchar_t suffix[LINE_MAX_LEN] = L"";
  remove_GXYZE(suffix, cur_line->str);
  wchar_t s1[LINE_MAX_LEN] = L"";
  wchar_t s2[LINE_MAX_LEN] = L"";
  wcscat(s1, prefix);
  wcscat(s2, prefix);
  wchar_t buf[LINE_MAX_LEN] = L"";
  Pos pos1 = bef_line->pos;
  Pos pos2 = cur_line->pos;
  if (cur_line->pos.XYZabs) {
    if (is_param(cur_line->str, L"X")) {
      pos1.X = bef_line->pos.X + (cur_line->pos.X - bef_line->pos.X) * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" X%.6f", pos1.X);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" X%.6f", pos2.X);
      wcscat(s2, buf);
    }
    if (is_param(cur_line->str, L"Y")) {
      pos1.Y = bef_line->pos.Y + (cur_line->pos.Y - bef_line->pos.Y) * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" Y%.6f", pos1.Y);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" Y%.6f", pos2.Y);
      wcscat(s2, buf);
    }
    if (is_param(cur_line->str, L"Z")) {
      pos1.Z = bef_line->pos.Z + (cur_line->pos.Z - bef_line->pos.Z) * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" Z%.6f", pos1.Z);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" Z%.6f", pos2.Z);
      wcscat(s2, buf);
    }
  } else {
    double dbuf = 0.0;
    if (get_param_ptr(cur_line->str, L"X", &dbuf)) {
      pos1.X += dbuf * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" X%.6f", dbuf * delta / full_time);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" X%.6f", dbuf * (full_time - delta) / full_time);
      wcscat(s2, buf);
    }
    if (get_param_ptr(cur_line->str, L"Y", &dbuf)) {
      pos1.Y += dbuf * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" Y%.6f", dbuf * delta / full_time);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" Y%.6f", dbuf * (full_time - delta) / full_time);
      wcscat(s2, buf);
    }
    if (get_param_ptr(cur_line->str, L"Z", &dbuf)) {
      pos1.Z += dbuf * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" Z%.6f", dbuf * delta / full_time);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" Z%.6f", dbuf * (full_time - delta) / full_time);
      wcscat(s2, buf);
    }
  }
  if (cur_line->pos.Eabs) {
    if (is_param(cur_line->str, L"E")) {
      pos1.E = bef_line->pos.E + (cur_line->pos.E - bef_line->pos.E) * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" E%.6f", pos1.E);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" E%.6f", pos2.E);
      wcscat(s2, buf);
    }
  } else {
    double dbuf = 0.0;
    if (get_param_ptr(cur_line->str, L"E", &dbuf)) {
      pos1.E += dbuf * delta / full_time;
      swprintf_s(buf, sizeof(buf), L" E%.6f", dbuf * delta / full_time);
      wcscat(s1, buf);
      swprintf_s(buf, sizeof(buf), L" E%.6f", dbuf * (full_time - delta) / full_time);
      wcscat(s2, buf);
    }
  }
  wcscat(s1, suffix);
  wcscat(s2, suffix);
  *out1 = malloc(sizeof(GcodeLine));
  (*out1)->str = malloc(sizeof(wchar_t) * wcslen(s1));
  if (*out1 == NULL || (*out1)->str == NULL)
    throw_error(ERR_ALLOC);
  wcscpy((*out1)->str, s1);
  (*out1)->pos = pos1;
  (*out1)->time = bef_line->time + (cur_line->time - bef_line->time) * delta / full_time;
  *out2 = malloc(sizeof(GcodeLine));
  (*out2)->str = malloc(sizeof(wchar_t) * wcslen(s2));
  if (*out2 == NULL || (*out2)->str == NULL)
    throw_error(ERR_ALLOC);
  wcscpy((*out2)->str, s2);
  (*out2)->pos = pos2;
  (*out2)->time = cur_line->time;
  return 1;
}

void remove_GXYZE(wchar_t* dest, wchar_t* src) {
  assert(dest != NULL && src != NULL);
  unsigned long i = 0;
  unsigned long j = 1;
  dest[0] = ' ';
  for (i = 0; src[i] != '\0' && src[i] != ';'; i++) {
    if (src[i] == 'G' || src[i] == 'X' || src[i] == 'Y' || src[i] == 'Z' || src[i] == 'E') {
      do {
        i++;
      } while (char_is_double(src[i]));
      i--;
    } else {
      if (!(dest[j - 1] == ' ' && src[i] == ' ')) {
        dest[j] = src[i];
        j++;
      }
    }
  }
  for (i = i; src[i] != '\0'; i++, j++) {
    dest[j] = src[i];
  }
  return;
}
#endif
