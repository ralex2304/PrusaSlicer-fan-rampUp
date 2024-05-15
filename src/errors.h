#ifndef ERRORS_H_
#define ERRORS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>

enum Errors {ERR_CONSOLE_ARGUMENTS, ERR_FILE_OPEN, ERR_FILE_READ, ERR_FILE_WRITE, ERR_ESTIMATE_LINES_NOT_FOUND, ERR_ALLOC};
enum Warnings {WARN_SPLIT_ERROR};

void throw_error(enum Errors err, const char* filename);

void throw_warning(enum Warnings warn, const wchar_t* line);

#endif
