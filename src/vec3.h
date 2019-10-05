#ifndef VEC3_H
#define VEC3_H

#include <ostream>

class vec3
{
	public:

	union {
		float _d[3];
		struct { float x, y, z; };
	};

	vec3 (float xx, float yy, float zz);
	vec3 (float xx, float yy);
	vec3 (float c);
	vec3 ();

	inline float& operator[] (int i) { return _d[i]; }

	inline vec3& operator+= (vec3 v);
	inline vec3& operator-= (vec3 v);
	inline vec3& operator*= (float c);
	inline vec3& operator/= (float c);

	vec3& norm ();
};

inline vec3 operator+ (vec3 one, vec3 other);
inline vec3 operator- (vec3 one, vec3 other);
inline vec3 operator* (vec3 v, float c);
inline vec3 operator* (float c, vec3 v);
inline vec3 operator/ (vec3 v, float c);

float abs (const vec3& v);

std::ostream& operator<< (std::ostream& s, vec3 v);
std::istream& operator>> (std::istream& s, vec3 v);

vec3 atovec3 (const char* s);
void atovec3 (const char* s, vec3& v);

vec3 atovec3 (const std::string& s);
void atovec3 (const std::string& s, vec3& v);

#endif // VEC3_H

