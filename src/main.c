#include <stdio.h>
#include "gcode.h"
#include "files.h"
#include "cvector.h"

//#define NDEBUG

cvector v_str;
cvector v_times;
cvector v_lines;
cvector v_fan_lines;
cvector v_symbols;
void process(cvector* strings, cvector* times, cvector* lines, cvector* fan_lines, double delay);
int fan_stack_proc(cvector* lines, cvector* fan_lines, const GcodeLine* line,
                   const unsigned long target_index, const double current_fan);
void init_vectors();

int main(int argc, char* argv[]) {
  if (argc != 3)
    throw_error(ERR_CONSOLE_ARGUMENTS);
  init_vectors();
  file_read_lines(argv[2], &v_str, &v_symbols, &v_times);
  process(&v_str, &v_times, &v_lines, &v_fan_lines, strtod(argv[1], NULL));
  file_write_lines(argv[2], &v_lines);
  return 0;
}

void process(cvector* strings, cvector* times, cvector* lines, cvector* fan_lines, double delay) {
  if (delay <= 0.0)
    throw_error(ERR_CONSOLE_ARGUMENTS);
  static double current_fan = 0;
  GcodeLine cur_line;
  gcode_init_line(&cur_line);
  for (unsigned long i = 0; i < strings->size; i++) {
    cur_line.str = *((wchar_t**)cvector_get(strings, i));
    cur_line.time += *((double*)cvector_get(times, i));
    gcode_parse_line(&cur_line);
    if (startswith(cur_line.str, L"M107")) {
      cvector_push(lines, &cur_line);
      unsigned long tmp = lines->size - 1;
      cvector_push(fan_lines, &tmp);
      current_fan = 0.0;
      continue;
    } else if (startswith(cur_line.str, L"M106")) {
      if (get_param(cur_line.str, L"S") <= current_fan) {
        cur_line.pos.fan_prev = current_fan;
        cvector_push(lines, &cur_line);
        current_fan = get_param(cur_line.str, L"S");
        unsigned long tmp = lines->size - 1;
        cvector_push(fan_lines, &tmp);
      } else {
        cur_line.pos.fan_prev = current_fan;
        current_fan = get_param(cur_line.str, L"S");
        double target_time = cur_line.time - delay;
        unsigned long target_index = find_line_by_time(lines, target_time);
        if (fan_stack_proc(lines, fan_lines, &cur_line, target_index, current_fan) == 0)
          continue;
        GcodeLine* bef_line;
        if (target_index == 0)
          bef_line = cvector_get(lines, 0);
        else
          bef_line = cvector_get(lines, target_index-1);
        GcodeLine* line1 = NULL;
        GcodeLine* line2 = NULL;
        if (target_time == bef_line->time || target_index == 0 
            || !gcode_split_line(cvector_get(lines, target_index), bef_line, target_time-bef_line->time, &line1, &line2)) {
          GcodeLine new_line = *bef_line;
          new_line.str = cur_line.str;
          cvector_insert(lines, target_index, &new_line);
          cvector_push(fan_lines, &target_index);
        } else {
          assert(line1 != NULL);
          assert(line2 != NULL);
          GcodeLine new_line = *line1;
          new_line.str = cur_line.str;
          cvector_set(lines, target_index, line2);
          cvector_insert(lines, target_index, &new_line);
          cvector_insert(lines, target_index, line1);
          free(line1);
          free(line2);
          target_index++;
          cvector_push(fan_lines, &target_index);
        }
      }
      continue;
    }
    cvector_push(lines, &cur_line);
  }
  return;
}

int fan_stack_proc(cvector* lines, cvector* fan_lines, const GcodeLine* line,
                   const unsigned long target_index, const double current_fan) {
  while (fan_lines->size && *((unsigned long*)cvector_get(fan_lines, fan_lines->size-1)) > target_index) {
    GcodeLine* fan_line = cvector_get(lines, *((unsigned long*)cvector_get(fan_lines, fan_lines->size-1)));
    assert(fan_line != NULL);
    if (startswith(fan_line->str, L"M107")) {
      fan_line->str = realloc(fan_line->str, 1 * sizeof(wchar_t));
      if (fan_line->str == NULL)
        throw_error(ERR_ALLOC);
      *fan_line->str = '\0';
      cvector_pop(fan_lines);
    } else {
      if (fan_line->pos.fan_prev >= current_fan) {
        fan_line->str = realloc(fan_line->str, (wcslen(line->str)+1) * sizeof(wchar_t));
        if (fan_line->str == NULL)
          throw_error(ERR_ALLOC);
        wcscpy(fan_line->str, line->str);
        free(line->str);
        return 0;
      } else {
        fan_line->str = realloc(fan_line->str, 1 * sizeof(wchar_t));
        if (fan_line->str == NULL)
          throw_error(ERR_ALLOC);
        *fan_line->str = '\0';
        cvector_pop(fan_lines);
      }
    }
  }
  return 1;
}

void init_vectors() {
  cvector_init(&v_str, sizeof(wchar_t*));
  cvector_init(&v_symbols, sizeof(wchar_t));
  cvector_init(&v_times, sizeof(double));
  cvector_init(&v_lines, sizeof(GcodeLine));
  cvector_init(&v_fan_lines, sizeof(unsigned long));
  return;
}