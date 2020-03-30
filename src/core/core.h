#ifndef CORE_H
#define CORE_H

#include "misc.h"
#include <queue>

extern bool must_quit;
extern int exit_code;
extern unsigned long long tick;

void update ();

/*
 * Entity format:
 * entity-class
 * 	key1 value1
 * 	key2 value2
 * 	! event-name delay target signal...
 * etc.
 * The line with the entity class must not start with whitespace;
 * the lines with key-value pairs must start with whitespace.
 * Those lines among key-value pairs which start with !
 *   are event handlers, which tell the entity to send a signal
 *   when something happens.
 */

void load_map (std::string path);
void init_core ();

/*
 * General purpose
 */

/*
 * Axially-aligned bounding box
 */
struct t_bound_box
{
	vec3 start;
	vec3 end;

	void expand (const vec3& pt);
	void expand (const t_bound_box& other);

	bool point_in (const vec3& pt) const;
	float volume () const;

	bool intersects (const t_bound_box& b) const;

	void intersect (const t_bound_box& b);
	void intersect_guarded (const t_bound_box& b);

	inline const float* data () const { return glm::value_ptr(start); }
};
static_assert(
	offsetof(t_bound_box, end) - offsetof(t_bound_box, start)
		== sizeof(vec3),
	"Alignment for t_bound_box is broken");

#endif // CORE_H
