#ifndef _STRINGS_H
#define _STRINGS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int startswith(const wchar_t* str, const wchar_t* x);
int get_param_ptr(const wchar_t* str, const wchar_t* x, double* const out);
double get_param(const wchar_t* str, const wchar_t* x);
int is_param(const wchar_t* str, const wchar_t* x);
int char_is_double(const wchar_t c);

int startswith(const wchar_t* str, const wchar_t* x) {
  return wcsncmp(x, str, wcslen(x)) == 0;
}

int get_param_ptr(const wchar_t* str, const wchar_t* x, double* const out) {
  if (str == NULL || x == NULL)
    return 0;
  int i = 0;
  int j = 0;
  while (str[i] != '\0' && str[i] != ';' && x[j] != '\0') {
    if(str[i] == x[j])
      j++;
    else
      j = 0;
    i++;
  }
  if (str[i] == '\0' || str[i] == ';' || !char_is_double(str[i]))
    return 0;
  *out = wcstod(str + i, NULL);
  return 1;
}

double get_param(const wchar_t* str, const wchar_t* x) {
  double out = 0.0;
  get_param_ptr(str, x, &out);
  return out;
}

int is_param(const wchar_t* str, const wchar_t* x) {
  if (str == NULL || x == NULL)
    return 0;
  int i = 0;
  int j = 0;
  while (str[i] != '\0' && str[i] != ';' && x[j] != '\0') {
    if (str[i] == x[j])
      j++;
    else
      j = 0;
    i++;
  }
  if (str[i] == '\0' || str[i] == ';' || !char_is_double(str[i]))
    return 0;
  return 1;
}

int char_is_double(const wchar_t c) {
  return c == '.' || c == ',' || c == '-' || ('0' <= c && c <= '9');
}
#endif
