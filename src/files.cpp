#include "files.h"

#include "errors.h"
#include "gcode.h"
#include "strings.h"


void file_read_lines(const char* filename, CVector* strings, CVector* times) {
	FILE* file = fopen(filename, "r");

	if(file == NULL)
		throw_error(ERR_FILE_OPEN, filename);

	wchar_t str[LINE_MAX_LEN] = L"";
	while(!feof(file)) {

		if (fgetws(str, LINE_MAX_LEN, file)) {

			if (startswith(str, TIME_START_STR))
				break;

			wchar_t* buf = (wchar_t*)calloc(wcslen(str) + 1, sizeof(wchar_t));
			wcscpy(buf, str);
			cvector_push(strings, &buf);
		}
	}

	if (feof(file))
		throw_error(ERR_ESTIMATE_LINES_NOT_FOUND, nullptr);

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

void file_write_lines(const char* filename, CVector* lines) {
	assert(filename);
	assert(lines);

	FILE* file = fopen(filename, "w");

	if(file == NULL)
		throw_error(ERR_FILE_OPEN, filename);

	fputws(INIT_LINE, file);

	for (unsigned long i = 0; i < lines->size; i++)
		if (((GcodeLine*)cvector_get(lines, i))->str != NULL)
			fputws(((GcodeLine*)cvector_get(lines, i))->str, file);

	if (ferror(file))
		throw_error(ERR_FILE_WRITE, filename);

	fclose(file);
}
