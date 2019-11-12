#ifndef ERROR_H
#define ERROR_H

#include <cstdarg>

[[noreturn]]
void fatal (const char* format, ...);

void warning (const char* format, ...);

#endif // ERROR_H
