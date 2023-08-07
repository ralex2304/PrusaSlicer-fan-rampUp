#ifndef _ERRORS_H
#define _ERRORS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

enum ERRORS {ERR_CONSOLE_ARGUMENTS, ERR_FILE_OPEN, ERR_ESTIMATE_LINES_NOT_FOUND};
enum WARNINGS {WARN_SPLIT_ERROR};

void throw_error(enum ERRORS ERR, ...);
void throw_warning(enum WARNINGS WARN, ...);

void throw_error(enum ERRORS ERR, ...){
  va_list ap;
  printf("Critical error:\n");
  switch(ERR){
  case ERR_CONSOLE_ARGUMENTS:
    printf("Wrong console arguments.\n");
    break;
  case ERR_FILE_OPEN:
    va_start(ap, ERR);
    printf("Can't open gcode file \"");
    printf("%s", (char*)va_arg(ap, char*));
    printf("\".\n");
    break;
  case ERR_ESTIMATE_LINES_NOT_FOUND:
    printf("No estimate lines found. Add klipper_estimator fork to postprocessors.\n");
    break;
  default:
    return;
  }
  printf("Press enter to exit");
  while(!getchar()){}
  exit(1);
  return;
}

void throw_warning(enum WARNINGS WARN, ...){
  va_list ap;
  printf("Warning:\n");
  switch(WARN){
  case WARN_SPLIT_ERROR:
    va_start(ap, WARN);
    printf("Can't split line. G0/G1 not found. Check klipper_estimator parameters.\n");
    wprintf(L"Line: %S", (wchar_t*)va_arg(ap, wchar_t*));
    printf("\n");
    break;
  default:
    return;
  }
  printf("Press enter to continue");
  while(!getchar()){}
}
#endif
