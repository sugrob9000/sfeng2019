#include "misc.h"
#include <cstdio>
#include <cstdlib>

[[noreturn]]
void fatal (const char* format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(stderr, "Fatal: ");
	if (format != nullptr)
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
	if (format != nullptr)
		vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(args);
}

void atovec3 (const char* s, vec3& v)
{
	int n = sscanf(s, "%f%f%f", &v.x, &v.y, &v.z);
	if (n == 1) {
		// "1" -> { 1, 1, 1 }
		v.y = v.x;
		v.z = v.x;
	}
}

void atovec3 (const std::string& s, vec3& v)
{
	atovec3(s.c_str(), v);
}

vec3 atovec3 (const char* s)
{
	vec3 r;
	atovec3(s, r);
	return r;
}


vec3 atovec3 (const std::string& s)
{
	vec3 r;
	atovec3(s, r);
	return r;
}


std::string vec3toa (const vec3& v)
{
	using std::to_string;
	return to_string(v.x) + ' ' + to_string(v.y) + ' ' + to_string(v.z);
}

vec3 min_components (const vec3& a, const vec3& b)
{
	using std::min;
	return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

vec3 max_components (const vec3& a, const vec3& b)
{
	using std::max;
	return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}


mat3 rotate_xyz (const vec3& angles)
{
	const vec3 c = glm::cos(angles);
	const vec3 s = glm::sin(angles);
	return mat3(1.0,   0.0, 0.0,
	            0.0,  c[0], s[0],
	            0.0, -s[0], c[0]) *
	       mat3(c[1], 0.0, s[1],
	             0.0, 1.0, 0.0,
	           -s[1], 0.0, c[1]) *
	       mat3(c[2], s[2], 0.0,
	           -s[2], c[2], 0.0,
	             0.0, 0.0,  1.0);
}

uint32_t hash_int32_vector (const std::vector<uint32_t>& v)
{
	uint32_t r = v.size();
	for (uint32_t i: v)
		r ^= i + 0x9e3779b9 + (r << 6) + (r >> 2);
	return r;
}
