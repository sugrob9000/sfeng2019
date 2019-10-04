#include "keyval.h"
#include "inc_general.h"

namespace core
{

const std::string t_ent_keyvals::none = "";

const std::string& t_ent_keyvals::operator[] (std::string s) const
{
	auto i = m.find(s);
	if (i == m.end())
		return none;
	return i->second;
}

template<>
void kv_read<vec3> (const std::string& s, vec3& v)
{
	sscanf(s.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
}

template<>
void kv_read<float> (const std::string& s, float& f)
{
	sscanf(s.c_str(), "%f", &f);
}

}
