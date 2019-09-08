#ifndef VEC3_H
#define VEC3_H

class vec3
{
	float _d[3];

	public:

	vec3 (float x, float y, float z);
	vec3 ();

	float& x() { return _d[0]; }
	float& y() { return _d[1]; }
	float& z() { return _d[2]; }

	float& operator[] (int i) { return _d[i]; }
};

vec3 operator+ (vec3 one, vec3 other);
vec3 operator- (vec3 one, vec3 other);
vec3 operator* (vec3 v, float c);
vec3 operator* (float c, vec3 v);

#endif // VEC3_H

