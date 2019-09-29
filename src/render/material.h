#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_general.h"

namespace render
{

class t_material
{
	public:

	t_material (std::string pathmtf);

	bool load_mtf (std::string path);
	void apply ();
};

}

#endif // MATERIAL_H
