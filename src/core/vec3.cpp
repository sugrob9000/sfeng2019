#include "vec3.h"

vec3::vec3 (float x, float y, float z)
{
	_d[0] = x;
	_d[1] = y;
	_d[2] = z;
}

vec3::vec3 ()
{
	_d[0] = 0.0;
	_d[1] = 0.0;
	_d[2] = 0.0;
}

vec3 operator+ (vec3 one, vec3 other)
{
	return vec3(
		one.x() + other.x(),
		one.y() + other.y(),
		one.z() + other.z() );
}

vec3 operator- (vec3 one, vec3 other)
{
	return vec3(
		one.x() - other.x(),
		one.y() - other.y(),
		one.z() - other.z() );
}

vec3 operator* (vec3 v, float c)
{
	return vec3(c * v.x(), c * v.y(), c * v.z());
}

vec3 operator* (float c, vec3 v)
{
	return vec3(c * v.x(), c * v.y(), c * v.z());
}

