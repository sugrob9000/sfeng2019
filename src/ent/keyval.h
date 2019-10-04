#ifndef KEYVAL_H
#define KEYVAL_H

#include "inc_general.h"

namespace core
{

struct t_ent_keyvals
{
	public:
	std::map<std::string, std::string> m;
	static const std::string none;

	const std::string& operator[] (std::string s) const;
};

template<typename T>
void kv_read (const std::string& s, T& par);

}

#endif // KEYVAL_H
