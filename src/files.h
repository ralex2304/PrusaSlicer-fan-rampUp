#ifndef _FILES_H
#define _FILES_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cvector.h"
#include "gcode.h"
#include "errors.h"

#define TIME_START_STR  L";TIME_ESTIMATE_POSTPROCESSING"
#define INIT_LINE       L";FanStartUp postprocess\n"

void file_read_lines(const char* filename, cvector* strings, cvector* symbols, cvector* times);
void file_write_lines(const char* filename, cvector* v);

void file_read_lines(const char* filename, cvector* strings, cvector* symbols, cvector* times) {
  FILE* file = fopen(filename, "r");
  if(file == NULL)
    throw_error(ERR_FILE_OPEN, filename);
  unsigned long i = 0;
  wchar_t str[LINE_MAX_LEN] = L"";
  while(!feof(file)) {
    if (fgetws(str, LINE_MAX_LEN, file)) {
      if (startswith(str, TIME_START_STR))
        break;
      wchar_t* buf = malloc((wcslen(str)+1) * sizeof(wchar_t));
      wcscpy(buf, str);
      cvector_push(strings, &buf);
    }
  }
  if (feof(file))
    throw_error(ERR_ESTIMATE_LINES_NOT_FOUND);
  while (!feof (file)) {
    if (fgetws(str, LINE_MAX_LEN, file)){
      double time = wcstod(str, NULL);
      cvector_push(times, &time);
    }
  }
  if (ferror(file))
    throw_error(ERR_FILE_READ, filename);
  fclose(file);
}

void file_write_lines(const char* filename, cvector* v) {
  FILE* file = fopen(filename, "w");
  if(file == NULL)
    throw_error(ERR_FILE_OPEN, filename);
  fputws(INIT_LINE, file);
  for (unsigned long i = 0; i < v->size; i++)
    if (((GcodeLine*)cvector_get(v, i))->str != NULL)
      fputws(((GcodeLine*)cvector_get(v, i))->str, file);
  if (ferror(file))
    throw_error(ERR_FILE_WRITE, filename);
  fclose(file);
}
#endif
