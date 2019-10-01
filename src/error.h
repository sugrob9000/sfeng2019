#ifndef ERROR_H
#define ERROR_H

#include <cstdarg>

namespace core
{

void fatal (const char* format, ...);
void warning (const char* format, ...);

}

#endif // ERROR_H
