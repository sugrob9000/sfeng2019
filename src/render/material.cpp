#include "render.h"
#include "material.h"
#include "inc_general.h"

namespace render
{

t_material::t_material (std::string mtfpath)
{
	load_mtf(mtfpath);
}

bool t_material::load_mtf (std::string path)
{
	return false;
}

void t_material::apply ()
{
	// TODO: apply shaders, etc.
}

}
