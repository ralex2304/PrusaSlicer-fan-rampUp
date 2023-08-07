#ifndef _FILES_H
#define _FILES_H
#include <stdio.h>
#include <stdlib.h>
#include "cvector.h"
#include "gcode.h"
#include "errors.h"

#define LINE_MAX_LEN 256
#define TIME_START_STR L";TIME_ESTIMATE_POSTPROCESSING"
#define INIT_LINE L";FanStartUp postprocess\n"
void file_read_lines(const char* filename, cvector* v);
void file_write_lines(const char* filename, cvector* v);

void file_read_lines(const char* filename, cvector* v){
  FILE* file = fopen(filename, "r");
  if(file == NULL)
    throw_error(ERR_FILE_OPEN, filename);
  GcodeLine line;
  gcode_init_line(&line);
  wchar_t str[LINE_MAX_LEN] = L"";
  while(!feof (file)) {
    if (fgetws(str, LINE_MAX_LEN, file)){
      if(startswith(str, TIME_START_STR))
        break;
      line.str = malloc((wcslen(str)+1) * sizeof(wchar_t));
      wcscpy(line.str, str);
      gcode_parse_line(&line);
      cvector_push(v, &line);
    }
  }
  if(feof(file))
    throw_error(ERR_ESTIMATE_LINES_NOT_FOUND);
  unsigned int i = 0;
  long double time = 0;
  while(!feof (file)) {
    if (fgetws(str, LINE_MAX_LEN, file)){
      time += wcstod(str, NULL);
      ((GcodeLine*) cvector_get(v, i))->time = time;
      i++;
    }
  }
  fclose(file);
}

void file_write_lines(const char* filename, cvector* v){
  FILE* file = fopen(filename, "w");
  if(file == NULL)
    throw_error(ERR_FILE_OPEN, filename);
  fputws(INIT_LINE, file);
  for(unsigned long i = 0; i < v->size; i++)
    fputws(((GcodeLine*)cvector_get(v, i))->str, file);
  fclose(file);
}
#endif
