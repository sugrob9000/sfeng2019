#ifndef KEYVAL_H
#define KEYVAL_H

#include "inc_general.h"

namespace core
{

struct t_ent_keyvals
{
	public:
	std::map<std::string, std::string> m;
	static std::string none;

	std::string& operator[] (std::string s);
};

}

#endif // KEYVAL_H
