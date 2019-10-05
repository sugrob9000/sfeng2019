#include "keyval.h"
#include "inc_general.h"

namespace core
{

std::string t_ent_keyvals::none = "";

std::string& t_ent_keyvals::operator[] (std::string s)
{
	auto i = m.find(s);
	if (i == m.end())
		return none;
	return i->second;
}

}
