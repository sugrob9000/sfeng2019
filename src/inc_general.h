#ifndef INC_GENERAL_H
#define INC_GENERAL_H

#include <cmath>
#include <iostream>
#include <fstream>

#include "vec3.h"
#include "error.h"

#define DEBUG_EXPR(expr)                          \
	do {                                      \
		std::cerr << #expr " = "          \
		          << (expr) << std::endl; \
	} while (false)

#define DEBUG_MSG(msg)                                    \
	do {                                              \
		std::cerr << "d: " << (msg) << std::endl; \
	} while (false)

/* Smallest power of 2 >= x */
#define CEIL_PO2(x) \
	(1 << (8 * sizeof((x)) - __builtin_clz((x)-1)))

/* Whether x is a power of 2 */
#define IS_PO2(x) (((x) & (x-1)) == 0)

constexpr float DEG_TO_RAD = M_PI / 180.0;
constexpr float RAD_TO_DEG = 180.0 / M_PI;

#endif // INC_GENERAL_H
