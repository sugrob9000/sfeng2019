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

	const std::string& operator[] (std::string s) const;
	void add (std::string key, std::string value);
};

}

#endif // KEYVAL_H
