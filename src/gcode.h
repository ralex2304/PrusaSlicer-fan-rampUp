#ifndef GCODE_H_
#define GCODE_H_
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>

#include "cvector.h"

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
unsigned long find_line_by_time(CVector* lines, const double target);
int gcode_split_line(GcodeLine* cur_line, GcodeLine* bef_line, double delta, GcodeLine** out1, GcodeLine** out2);
void remove_GXYZE(wchar_t* dest, wchar_t* src);

#endif
