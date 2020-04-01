#include "ent/lights.h"
#include "input/cmds.h"
#include "render/framebuffer.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/vis.h"
#include "render/gbuffer.h"

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
	lights.push_back(this);
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
	KV_TRY_GET(kv["reach"],
		reach = atof(val.c_str());,
		reach = 2000.0; );
	KV_TRY_GET(kv["rgb"],
		atovec3(val, rgb);,
		rgb = vec3(0.5); );
}

void e_light::render () const { }

t_bound_box e_light::get_bbox () const { return { }; }

void e_light::view () const
{
	using namespace glm;

	render_ctx.proj = perspective(radians(2.0f * cone_angle),
			1.0f, LIGHT_Z_NEAR, reach);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);
}



/* ======================================== */

/* Depth maps from lights' perspective */
constexpr int lspace_resolution = 1024;
t_fbo lspace_fbo;

/* Screen space shadow maps */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;

static GLuint lighting_program;

void init_lighting ()
{
	constexpr int s = lspace_resolution;

	lspace_fbo.make()
		.attach_color(make_tex2d(s, s, GL_R32F))
		.attach_depth(make_rbo(s, s, GL_DEPTH_COMPONENT))
		.assert_complete();

	int sw = sdlctx.res_x;
	int sh = sdlctx.res_y;
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(sw, sh, GL_RGB16F))
			.assert_complete();
		sspace_add_buffer(sspace_fbo[i]);
	}

	lighting_program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light") });

	glUseProgram(lighting_program);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 0);
	glUniform1i(UNIFORM_LOC_DEPTH_MAP, 1);

	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_POS, 2);
	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_NORM, 3);
	glUniform1i(UNIFORM_LOC_GBUFFER_SCREEN_DEPTH, 4);
}

void light_init_material ()
{
	glUniform1i(UNIFORM_LOC_LIGHTMAP_DIFFUSE, 0);
	glUniform1i(UNIFORM_LOC_LIGHTMAP_SPECULAR, 1);
}

void light_apply_material ()
{
	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo].color[0]->id);
	// TODO: specular
}


vec3 e_light::unif_pos;
vec3 e_light::unif_rgb;
mat4 e_light::unif_view;
vec2 e_light::unif_cascade_bounds[2];



/* Returns: whether this light is potentially visible */
static bool fill_depth_map (const e_light* l)
{
	static t_bound_box view_bounds =
		{ { -1.0, -1.0, 0.0 }, { 1.0, 1.0, 1.0 } };

	// aliases
	mat4& proj = render_ctx.proj;
	mat4& view = render_ctx.view;

	// whether center of light is visible by camera
	vec4 camspace = proj * view * vec4(l->pos, 1.0);
	bool inside_camera = view_bounds.point_in(camspace / camspace.w);

	matrix_restorer restore(render_ctx);
	l->view();

	t_bound_box lspace_bounds;
	if (inside_camera)
		lspace_bounds = { { -1.0, -1.0, 0.0 }, { 1.0, 1.0, 0.0 } };
	else
		lspace_bounds = { vec3(INFINITY), vec3(-INFINITY) };

	std::array<vec3, 4> planes[2] =
		{ camera.corner_points(camera.z_near),
		  camera.corner_points(camera.z_far) };

	for (int i = 0; i < 8; i++) {
		vec4 v = proj * view * vec4(planes[i / 4][i % 4], 1.0);
		lspace_bounds.expand(v / std::abs(v.w));
	}

	// clip to what light can see
	lspace_bounds.intersect_guarded(view_bounds);

	if (lspace_bounds.volume() <= 0.0) {
		// cannot see this light
		return false;
	}

	// ensure everything between the light and the slice is rendered
	lspace_bounds.start.z = 0.0;

	// save values for use as uniforms in later pass
	e_light::unif_view = proj * view;
	e_light::unif_pos = l->pos;
	e_light::unif_rgb = l->rgb;
	e_light::unif_cascade_bounds[0] = lspace_bounds.start;
	e_light::unif_cascade_bounds[1] = lspace_bounds.end;

	vec3 center = -(lspace_bounds.start + lspace_bounds.end) * 0.5f;
	vec3 scale = vec3(2.0) / (lspace_bounds.end - lspace_bounds.start);
	center.z = 0.0;
	scale.z *= 0.5;

	mat4 subfrustum = glm::translate(glm::scale(mat4(1.0), scale), center);
	render_ctx.proj = subfrustum * render_ctx.proj;

	lspace_fbo.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	material_barrier();
	l->vis.render();

	return true;
}

static void lighting_pass ()
{
	current_sspace_fbo ^= 1;
	sspace_fbo[current_sspace_fbo].apply();

	glUseProgram(lighting_program);

	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo ^ 1].color[0]->id);
	bind_tex2d_to_slot(1, lspace_fbo.color[0]->id);

	bind_tex2d_to_slot(2, gbuf_fbo.color[GBUF_SLOT_WORLD_POS]->id);
	bind_tex2d_to_slot(3, gbuf_fbo.color[GBUF_SLOT_WORLD_NORM]->id);
	bind_tex2d_to_slot(4, gbuf_fbo.depth->id);

	glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, value_ptr(e_light::unif_pos));
	glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, value_ptr(e_light::unif_rgb));
	glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEW, 1, false,
			value_ptr(e_light::unif_view));

	glUniform2fv(UNIFORM_LOC_LIGHT_CASCADE, 2,
			value_ptr(e_light::unif_cascade_bounds[0]));

	gbuffer_pass();
}

void compute_lighting ()
{
	sspace_fbo[0].apply();
	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	current_sspace_fbo = 0;

	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;
	for (e_light* l: lights) {
		if (fill_depth_map(l))
			lighting_pass();
	}
}

vec3 ambient = vec3(0);
COMMAND_ROUTINE (light_ambience)
{
	if (ev != PRESS)
		return;
	switch (args.size()) {
	case 1:
		ambient = vec3(atof(args[0].c_str()));
		break;
	case 3:
		ambient = vec3(atof(args[0].c_str()),
		               atof(args[1].c_str()),
		               atof(args[2].c_str()));
		break;
	default:
		return;
	}
}
