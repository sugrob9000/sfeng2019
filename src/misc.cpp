#include "misc.h"

std::ostream& operator<< (std::ostream& s, const vec3& v)
{
	return (s << v.x << ' ' << v.y << ' ' << v.z);
}

std::istream& operator>> (std::istream& s, vec3& v)
{
	return (s >> v.x >> v.y >> v.z);
}

vec3 atovec3 (const char* s)
{
	vec3 r;
	sscanf(s, "%f %f %f", &r.x, &r.y, &r.z);
	return r;
}

void atovec3 (const char* s, vec3& v)
{
	sscanf(s, "%f %f %f", &v.x, &v.y, &v.z);
}

vec3 atovec3 (const std::string& s)
{
	vec3 r;
	sscanf(s.c_str(), "%f %f %f", &r.x, &r.y, &r.z);
	return r;
}

void atovec3 (const std::string& s, vec3& v)
{
	sscanf(s.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
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

