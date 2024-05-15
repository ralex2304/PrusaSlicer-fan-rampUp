#ifndef FILES_H_
#define FILES_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cvector.h"

#define TIME_START_STR  L";TIME_ESTIMATE_POSTPROCESSING"
#define INIT_LINE       L";FanStartUp postprocess\n"

void file_read_lines(const char* filename, CVector* strings, CVector* times);

void file_write_lines(const char* filename, CVector* v);

#endif
