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

	float& operator[] (int i) { return _d[i]; }
	const float& operator[] (int i) const { return _d[i]; }

	float* data () { return _d; }
	const float* data () const { return _d; }

	vec3& operator+= (vec3 v);
	vec3& operator-= (vec3 v);
	vec3& operator*= (float c);
	vec3& operator/= (float c);

	vec3& norm ();
};

static_assert(offsetof(vec3, x) == offsetof(vec3, _d[0])
           && offsetof(vec3, y) == offsetof(vec3, _d[1])
           && offsetof(vec3, z) == offsetof(vec3, _d[2]),
	   "Memory alignment for vec3 is broken");

vec3 operator* (vec3 v, float c);
vec3 operator* (float c, vec3 v);
vec3 operator/ (vec3 v, float c);

/*
 * Component-wise
 */
vec3 operator+ (vec3 a, vec3 b);
vec3 operator- (vec3 a, vec3 b);
vec3 operator* (vec3 a, vec3 b);
vec3 operator/ (vec3 a, vec3 b);
vec3 min (vec3 a, vec3 b);
vec3 max (vec3 a, vec3 b);

/*
 * Unary
 */
vec3 operator- (vec3 v);
vec3 operator+ (vec3 v);

float abs (const vec3& v);

std::ostream& operator<< (std::ostream& s, vec3 v);
std::istream& operator>> (std::istream& s, vec3& v);

vec3 atovec3 (const char* s);
void atovec3 (const char* s, vec3& v);

vec3 atovec3 (const std::string& s);
void atovec3 (const std::string& s, vec3& v);

bool operator< (const vec3& a, const vec3& b);
bool operator== (const vec3& a, const vec3& b);

#endif // VEC3_H

