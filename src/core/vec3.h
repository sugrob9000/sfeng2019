#ifndef VEC3_H
#define VEC3_H

class vec3
{
	public:

	union {
		float _d[3];
		struct { float x, y, z; };
	};

	vec3 (float xx, float yy, float zz);
	vec3 ();

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

