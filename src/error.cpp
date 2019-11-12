#include "error.h"
#include <cstdio>
#include <cstdlib>

[[noreturn]]
void fatal (const char* format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(stderr, "Fatal: ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);

	// cannot use fancy facilities like must_quit
	// since we must be able to fatal() out of anywhere
	exit(EXIT_FAILURE);
}

void warning (const char* format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(stderr, "Warning: ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(args);
}
