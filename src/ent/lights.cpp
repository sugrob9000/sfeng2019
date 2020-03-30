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
	// render_ctx.proj = ortho(-100.0f, 100.0f, -100.0f, 100.0f, LIGHT_Z_NEAR, 2000.0f);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);
}



/* ======================================== */

mat4 e_light::unif_view;
vec3 e_light::unif_pos;
vec3 e_light::unif_rgb;

/* Depth maps from lights' perspective */
constexpr int lspace_samples = 4;
constexpr int lspace_resolution = 1024;

/* Render to multisampled, then blit into regular for sampling */
t_fbo lspace_fbo_ms;
t_fbo lspace_fbo;

/* Screen space shadow maps */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;

constexpr int cascades_num = 1;
struct t_plane { glm::vec4 points[4]; };
std::vector<t_plane> planes;

void get_planes ()
{
	for (int i = 0; i < cascades_num + 1; i++) {
		float l = camera.z_near + (float)i * 
			(200.0 - camera.z_near) / cascades_num;
		t_plane plane;

		for (int j = 0; j < 4; j++) {
			plane.points[j].z = -l;
			plane.points[j].w = 1.0;
		}

		plane.points[1].x = plane.points[2].x = 
			l * tan(glm::radians(camera.fov / 2));
		plane.points[0].x = plane.points[3].x = 
			-l * tan(glm::radians(camera.fov / 2));

		plane.points[0].y = plane.points[1].y = 
			-l * tan(glm::radians(camera.fov / 2)) / camera.aspect;
		plane.points[2].y = plane.points[3].y = 
			l * tan(glm::radians(camera.fov / 2)) / camera.aspect;

		planes.push_back(plane);
	}
}

static GLuint lighting_program;

void init_lighting ()
{
	constexpr int s = lspace_resolution;

	lspace_fbo_ms.make()
		.attach_color(make_rbo_msaa(s, s, GL_R16F, lspace_samples))
		.attach_depth(make_rbo_msaa(
			s, s, GL_DEPTH_COMPONENT, lspace_samples))
		.assert_complete();

	lspace_fbo.make()
		.attach_color(make_tex2d(s, s, GL_R16F))
		.assert_complete();

	int sw = sdlctx.res_x;
	int sh = sdlctx.res_y;
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(sw, sh, GL_RGB16F))
			.assert_complete();
		sspace_add_buffer(sspace_fbo[i]);
	}

	get_planes();

	lighting_program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light") });

	glUseProgram(lighting_program);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 0);
	glUniform1i(UNIFORM_LOC_DEPTH_MAP, 1);
	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_POS, 2);
	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_NORM, 3);
}

void light_init_material ()
{
	glUniform1i(UNIFORM_LOC_LIGHTMAP_DIFFUSE, 0);
	glUniform1i(UNIFORM_LOC_LIGHTMAP_SPECULAR, 1);
}

vec3 min;
vec3 max;

void light_apply_material ()
{
	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo].color[0]->id);


	using glm::value_ptr;
	glUniform3fv(202, 3, value_ptr(min));
	glUniform3fv(205, 3, value_ptr(max));
	// TODO: specular
}

vec3 get_min (vec3 min, glm::vec4 point)
{
	vec3 m;

	if (min.x > point.x)
		m.x = point.x;
	else
		m.x = min.x;
	if (min.y > point.y)
		m.y = point.y;
	else
		m.y = min.y;
	if (min.z > point.z)
		m.z = point.z;
	else
		m.z = min.z;

	return m;
}

vec3 get_max (vec3 max, glm::vec4 point)
{
	vec3 m;

	if (max.x < point.x)
		m.x = point.x;
	else
		m.x = max.x;
	if (max.y < point.y)
		m.y = point.y;
	else
		m.y = max.y;
	if (max.z < point.z)
		m.z = point.z;
	else
		m.z = max.z;

	return m;
}

static void fill_depth_map (const e_light* l)
{
	lspace_fbo_ms.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	mat4 restore_proj = render_ctx.proj;
	mat4 restore_view = render_ctx.view;
	mat4 restore_model = render_ctx.model;

	l->view();

	mat4 cascades_stretch[cascades_num];

	for (int i = 0; i < cascades_num; i++) {

		glm::vec4 p = planes[i].points[0];
		p = glm::inverse(camera.view) * p;
		p = render_ctx.view * p;

		min.x = p.x;	min.y = p.y;	min.z = p.z;
		max.x = p.x;	max.y = p.y;	max.z = p.z;

		for (int j = 1; j < 4; j++) {
			glm::vec4 point = planes[i].points[j];
			point = glm::inverse(camera.view) * point;
			point = render_ctx.view * point;

			min = get_min(min, point);
			max = get_max(max, point);
		}

		for (int j = 0; j < 4; j++) {
			glm::vec4 point = planes[i + 1].points[j];
			point = glm::inverse(camera.view) * point;
			point = render_ctx.view * point;

			min = get_min(min, point);
			max = get_max(max, point);
		}

		vec3 t = max + min;
		t /= 2.0;
		vec3 s = max - min;
		s /= 2.0;

		cascades_stretch[0] = glm::translate(glm::mat4(1.0), t);
		cascades_stretch[0] = glm::scale(cascades_stretch[i], s);
	}

	DEBUG_EXPR(min);
	DEBUG_EXPR(max);

	mat4 res = render_ctx.proj;

	render_ctx.proj = cascades_stretch[0] * render_ctx.proj;
	// DEBUG_EXPR(l->name);
	// for (int i = 0; i < 4; i++) {
	// 	glm::vec4 point = planes[0].points[i];
	// 	point = glm::inverse(camera.view) * point;
	// 	point = render_ctx.view * point;
	// }
	// DEBUG_EXPR(point);

	l->vis.render();

	render_ctx.proj = res;

	e_light::unif_view = render_ctx.proj *
			render_ctx.view * render_ctx.model;
	e_light::unif_pos = l->pos;
	e_light::unif_rgb = l->rgb;

	render_ctx.proj = restore_proj;
	render_ctx.view = restore_view;
	render_ctx.model = restore_model;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, lspace_fbo_ms.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lspace_fbo.id);
	glBlitFramebuffer(0, 0, lspace_resolution, lspace_resolution,
	                  0, 0, lspace_resolution, lspace_resolution,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

static void lighting_pass ()
{
	current_sspace_fbo ^= 1;
	sspace_fbo[current_sspace_fbo].apply();

	glUseProgram(lighting_program);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo ^ 1].color[0]->id);
	bind_tex2d_to_slot(1, lspace_fbo.color[0]->id);
	bind_tex2d_to_slot(2, gbuf_fbo.color[GBUF_SLOT_WORLD_POS]->id);
	bind_tex2d_to_slot(3, gbuf_fbo.color[GBUF_SLOT_WORLD_NORM]->id);

	glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, value_ptr(e_light::unif_pos));
	glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, value_ptr(e_light::unif_rgb));
	glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEW, 1, false,
			value_ptr(e_light::unif_view));

	gbuffer_pass();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void compute_lighting ()
{
	sspace_fbo[0].apply();
	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	current_sspace_fbo = 0;

	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;
	for (e_light* l: lights) {
		fill_depth_map(l);
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
