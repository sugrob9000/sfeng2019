#ifndef MISC_H
#define MISC_H

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <vector>

[[noreturn]]
void fatal (const char* format, ...);
void warning (const char* format, ...);


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
#define IS_PO2(x) (((x) & ((x)-1)) == 0)


/*
 * GLM library bindings
 */
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


template<int N, class S, glm::qualifier Q>
std::ostream& operator<< (std::ostream& s, const glm::vec<N, S, Q>& v)
{
	for (int i = 0; i < N-1; i++)
		s << v[i] << ' ';
	return s << v[N-1];
}

template <int N, class S, glm::qualifier Q>
std::istream& operator>> (std::istream& s, glm::vec<N, S, Q>& v)
{
	for (int i = 0; i < N; i++)
		s >> v[i];
	return s;
}

vec3 atovec3 (const char* s);
void atovec3 (const char* s, vec3& v);
vec3 atovec3 (const std::string& s);
void atovec3 (const std::string& s, vec3& v);

std::string vec3toa (const vec3& v);

/* Component-wise min & max */
vec3 min_components (const vec3& a, const vec3& b);
vec3 max_components (const vec3& a, const vec3& b);


/* Rotation matrix for Euler angles - just x, then y, then z */
mat3 rotate_xyz (const vec3& angles);
inline mat4 rotate_xyz_4x4 (const vec3& a) { return mat4(rotate_xyz(a)); }


/* Clear a std::vector with guaranteed deallocation of data */
template <class T>
void vector_clear_dealloc (std::vector<T>& v)
{
	std::vector<T>().swap(v);
}

/* Hash a vector of integers */
uint32_t hash_int32_vector (const std::vector<uint32_t>& v);


/* Floor/ceil to nearest multiple of */
inline float floor_step (float a, float st) { return st * floor(a / st); }
inline float ceil_step (float a, float st) { return st * ceil(a / st); }

/*
 * Be able to component-wise "compare" vectors to
 * create maps of them, etc. Not very meaningful otheriwse
 */
template <int N, class S, glm::qualifier Q>
bool operator< (const glm::vec<N, S, Q>& a, const glm::vec<N, S, Q>& b)
{
	for (int i = 0; i < N; i++) {
		if (a[i] != b[i])
			return a[i] < b[i];
	}
	return false;
}

template <int N, class S, glm::qualifier Q>
bool operator== (const glm::vec<N, S, Q>& a, const glm::vec<N, S, Q>& b)
{
	for (int i = 0; i < N; i++) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}


/*
 * Restorer: use RAII to restore an object to
 * its original state after scope ends, for example:
 *
 * camera1.apply();
 * {
 * 	restorer rest(render_ctx);
 * 	camera2.apply();
 * 	// do rendering from the viewpoint of camera1
 * }
 * // camera1 is applied now
 */

template <class T> struct restorer
{
	T* ptr;
	T original_value;

	restorer (T& obj): ptr(&obj), original_value(obj) { }
	~restorer () { *ptr = original_value; }
};


#endif // MISC_H
