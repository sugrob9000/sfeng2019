#ifndef VEC3_H
#define VEC3_H

#include <ostream>

struct vec3
{
	union {
		float _d[3];
		struct { float x, y, z; };
	};

	vec3 (float xx, float yy, float zz);
	vec3 (float xx, float yy);
	vec3 (float c);
	vec3 ();

	inline float& operator[] (int i) { return _d[i]; }

	vec3& operator+= (vec3 v);
	vec3& operator-= (vec3 v);
	vec3& operator*= (float c);
	vec3& operator/= (float c);

	vec3& norm ();
};

static_assert(offsetof(vec3, x) == offsetof(vec3, _d[0]));
static_assert(offsetof(vec3, y) == offsetof(vec3, _d[1]));
static_assert(offsetof(vec3, z) == offsetof(vec3, _d[2]));

vec3 operator+ (vec3 one, vec3 other);
vec3 operator- (vec3 one, vec3 other);
vec3 operator* (vec3 v, float c);
vec3 operator* (float c, vec3 v);
vec3 operator/ (vec3 v, float c);

float abs (const vec3& v);

std::ostream& operator<< (std::ostream& s, vec3 v);
std::istream& operator>> (std::istream& s, vec3 v);

vec3 atovec3 (const char* s);
void atovec3 (const char* s, vec3& v);

vec3 atovec3 (const std::string& s);
void atovec3 (const std::string& s, vec3& v);

#endif // VEC3_H

