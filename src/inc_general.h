#ifndef INC_GENERAL_H
#define INC_GENERAL_H

#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <unordered_map>

#include "vec3.h"
#include "error.h"

#define DEBUG_OUT_EXPR(expr) \
	do { \
		std::cerr << #expr " = " \
		          << (expr) << std::endl; \
	} while (false)


#endif // INC_GENERAL_H
