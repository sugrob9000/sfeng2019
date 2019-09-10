#include "vec3.h"

vec3::vec3 (float xx, float yy, float zz)
{
	x = xx;
	y = yy;
	z = zz;
}

vec3::vec3 ()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

vec3 operator+ (vec3 one, vec3 other)
{
	return vec3(
		one.x + other.x,
		one.y + other.y,
		one.z + other.z );
}

vec3 operator- (vec3 one, vec3 other)
{
	return vec3(
		one.x - other.x,
		one.y - other.y,
		one.z - other.z );
}

vec3 operator* (vec3 v, float c)
{
	return vec3(c * v.x, c * v.y, c * v.z);
}

vec3 operator* (float c, vec3 v)
{
	return vec3(c * v.x, c * v.y, c * v.z);
}

vec3 operator/ (vec3 v, float c)
{
	return vec3(v.x / c, v.y / c, v.z / c);
}

vec3& vec3::operator+= (vec3 v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

vec3& vec3::operator-= (vec3 v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

vec3& vec3::operator*= (float c)
{
	x *= c;
	y *= c;
	z *= c;
	return *this;
}

vec3& vec3::operator/= (float c)
{
	x /= c;
	y /= c;
	z /= c;
	return *this;
}

