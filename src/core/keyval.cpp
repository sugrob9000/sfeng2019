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

void t_ent_keyvals::add (std::string key, std::string value)
{
	m[key] = value;
}

}
