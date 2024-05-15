#ifndef STRINGS_H_
#define STRINGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

int get_param_ptr(const wchar_t* str, const wchar_t* x, double* const out);

double get_param(const wchar_t* str, const wchar_t* x);

int is_param(const wchar_t* str, const wchar_t* x);

int startswith(const wchar_t* str, const wchar_t* x);

int char_is_double(const wchar_t c);

#endif
