#include "ent/lights.h"
#include "render/render.h"
#include "render/light/cone.h"
#include "render/light/sun.h"

std::vector<e_light_cone*> lights;

/* ======================== e_light_cone code ======================== */

SIG_HANDLER (light_cone, setcolor)
{
	atovec3(arg, ent->rgb);
}

SIG_HANDLER (light_cone, setcone)
{
	float cone = atof(arg.c_str());
	if (cone > 0.0 && cone < 180.0)
		ent->cone_angle = cone;
}

FILL_IO_DATA (light_cone)
{
	BASIC_SIG_HANDLERS(light_cone);
	SET_SIG_HANDLER(light_cone, setcolor);
	SET_SIG_HANDLER(light_cone, setcone);
}

e_light_cone::e_light_cone ()
{
	lights_cone.push_back(this);
}

e_light_cone::~e_light_cone ()
{
	for (e_light_cone*& p: lights_cone) {
		if (p == this) {
			p = lights_cone.back();
			lights_cone.pop_back();
			break;
		}
	}
}


void e_light_cone::moved ()
{
	e_base::moved();

	// update visible set
	view();
	vis.fill();
}


void e_light_cone::apply_keyvals (const t_ent_keyvals& kv)
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

void e_light_cone::think () { }
void e_light_cone::render () const { }
t_bound_box e_light_cone::get_bbox () const { return { }; }

void e_light_cone::view () const
{
	using namespace glm;

	render_ctx.proj = perspective(radians(2.0f * cone_angle),
			1.0f, near_plane, reach);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);

	render_ctx.eye_pos = pos;
}

/* ======================== e_light_sun code ======================== */

SIG_HANDLER (light_sun, setcolor)
{
	atovec3(arg, ent->rgb);
}

FILL_IO_DATA (light_sun)
{
	BASIC_SIG_HANDLERS(light_sun);
	SET_SIG_HANDLER(light_sun, setcolor);
}

void e_light_sun::think () { }
void e_light_sun::render () const { }
t_bound_box e_light_sun::get_bbox () const { return { }; }

void e_light_sun::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["distance"],
		distance = atof(val.c_str());,
		distance = 2000.0; );
	KV_TRY_GET(kv["rgb"],
		atovec3(val, rgb),
		rgb = vec3(0.5); );
}

e_light_sun::e_light_sun ()
{
	lights_sun.push_back(this);
}

e_light_sun::~e_light_sun ()
{
	for (e_light_sun*& p: lights_sun) {
		if (p == this) {
			p = lights_sun.back();
			lights_sun.pop_back();
			break;
		}
	}
}
