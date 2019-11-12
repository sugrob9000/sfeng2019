#ifndef KEYVAL_H
#define KEYVAL_H

#include "inc_general.h"
#include <map>

struct t_ent_keyvals
{
	std::map<std::string, std::string> m;
	static const std::string none;

	const std::string& operator[] (std::string s) const;
	void add (std::string key, std::string value);
	void clear ();
};

/*
 * Try to get a value from a t_ent_keyvals,
 * run code in the parameter if_there if it's there, or
 * run code in the parameter if_not_there if it's not
 * Example:
 *
 * const t_ent_keyvals& kv = ...;
 * KV_TRY_GET(kv["pos"],
 *		atovec3(val, pos); ,
 *		pos = vec3(0.0, 0.0, 0.0); );
 */
#define KV_TRY_GET(kvv, if_there, if_not_there) \
	do {                                    \
		const std::string& val = kvv;   \
		if (val.empty()) {              \
			if_not_there;           \
		} else {                        \
			if_there;               \
		}                               \
	} while (false)

#endif // KEYVAL_H
