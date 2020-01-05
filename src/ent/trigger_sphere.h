#ifndef ENT_TRIGGER_SPHERE_H
#define ENT_TRIGGER_SPHERE_H

#include "core/entity.h"

class e_trigger_sphere: public e_base
{
	public:

	float radius;
	bool touching;

	ENT_MEMBERS (trigger_sphere)

	constexpr static float DEFAULT_RADIUS = 100.0;
};

#endif // ENT_TRIGGER_SPHERE_H
