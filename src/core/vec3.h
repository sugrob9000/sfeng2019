#ifndef VEC3_H
#define VEC3_H

class vec3
{
	float _d[3];

	public:

	vec3 (float x, float y, float z);
	vec3 ();

	inline float& x() { return _d[0]; }
	inline float& y() { return _d[1]; }
	inline float& z() { return _d[2]; }

	inline float& operator[] (int i) { return _d[i]; }

	inline vec3& operator+= (vec3 v);
	inline vec3& operator-= (vec3 v);
	inline vec3& operator*= (float c);
	inline vec3& operator/= (float c);
};

inline vec3 operator+ (vec3 one, vec3 other);
inline vec3 operator- (vec3 one, vec3 other);
inline vec3 operator* (vec3 v, float c);
inline vec3 operator* (float c, vec3 v);
inline vec3 operator/ (vec3 v, float c);

#endif // VEC3_H

