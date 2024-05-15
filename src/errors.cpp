#include "errors.h"

#include <assert.h>

void throw_error(enum Errors ERR, const char* filename) {
	// filename can be nullptr

	fprintf(stderr, "Critical error:\n");
	switch (ERR) {
		case ERR_CONSOLE_ARGUMENTS:
			printf("Wrong console arguments.\n");
			break;

		case ERR_FILE_OPEN:
			fprintf(stderr, "Can't open gcode file \"%s\".\n", filename);
			break;

		case ERR_FILE_READ:
			fprintf(stderr, "Can't read from gcode file \"%s\".\n", filename);
			break;

		case ERR_FILE_WRITE:
			fprintf(stderr, "Can't write gcode file \"%s\".\n", filename);
			break;

		case ERR_ESTIMATE_LINES_NOT_FOUND:
			fprintf(stderr, "No estimate lines found. Add klipper_estimator fork to postprocessors.\n");
			break;

		case ERR_ALLOC:
			fprintf(stderr, "Memory allocation error. Probably not enough RAM.\n");

		default:
			return;
	}
	printf("Press enter to exit");
	while (!getchar()) {}
	exit(1);
	return;
}

void throw_warning(enum Warnings warn, const wchar_t* line) {
	assert(line);

	fprintf(stderr, "Warning:\n");
	switch (warn) {
		case WARN_SPLIT_ERROR:
			fprintf(stderr, "Can't split line. G0/G1 not found. Check klipper_estimator parameters.\n");
			fwprintf(stderr, L"Line: %S\n", line);
			break;

		default:
			return;
	}
	printf("Press enter to continue");
	while (!getchar()) {}
}
