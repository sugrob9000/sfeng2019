#ifndef ERROR_H
#define ERROR_H

#include "core.h"

namespace core
{

void error (const std::string& msg, bool fatal);
void warn (const std::string& msg);

}

#endif // ERROR_H
