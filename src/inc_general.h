#ifndef INC_GENERAL_H
#define INC_GENERAL_H

#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <unordered_map>
#include <cmath>

#include "vec3.h"
#include "error.h"

#define DEBUG_EXPR(expr)                          \
	do {                                      \
		std::cerr << #expr " = "          \
		          << (expr) << std::endl; \
	} while (false)

constexpr float DEG_TO_RAD = M_PI / 180.0;
constexpr float RAD_TO_DEG = 180.0 / M_PI;

#endif // INC_GENERAL_H
