#ifndef ERROR_H
#define ERROR_H

#include "core.h"

namespace core
{

void error (std::string msg, bool fatal);
void warn (std::string msg);

}

#endif // ERROR_H
