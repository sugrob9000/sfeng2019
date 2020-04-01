#include "ent/lights.h"
#include "render/render.h"
#include "render/light/cone.h"

std::vector<e_light*> lights;

/* ============= e_light code ============= */

SIG_HANDLER (light, setcolor)
{
	atovec3(arg, ent->rgb);
}

SIG_HANDLER (light, setcone)
{
	float cone = atof(arg.c_str());
	if (cone > 0.0 && cone < 180.0)
		ent->cone_angle = cone;
}

FILL_IO_DATA (light)
{
	BASIC_SIG_HANDLERS(light);
	SET_SIG_HANDLER(light, setcolor);
	SET_SIG_HANDLER(light, setcone);
}

e_light::e_light ()
{
	cone_lights.push_back(this);
}

void e_light::moved ()
{
	e_base::moved();

	// update visible set
	view();
	vis.fill(pos);
}

void e_light::think () { }

void e_light::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["cone"],
		cone_angle = atof(val.c_str());,
		cone_angle = 60.0; );
	KV_TRY_GET(kv["rgb"],
		atovec3(val, rgb);,
		rgb = vec3(0.5); );

	KV_TRY_GET(kv["reach"],
		reach = atof(val.c_str());,
		reach = 2000.0; );
	KV_TRY_GET(kv["near"],
		near_plane = atof(val.c_str()),
		near_plane = 1.0; );
}

void e_light::render () const { }

t_bound_box e_light::get_bbox () const { return { }; }

void e_light::view () const
{
	using namespace glm;

	render_ctx.proj = perspective(radians(2.0f * cone_angle),
			1.0f, near_plane, reach);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);
}
