#include "inc.h"

#include <fstream>

SDL_Window* sdlwindow;
SDL_GLContext sdlcontext;

int resolution_x = 640;
int resolution_y = 480;

float offset_x = 0.0;
float offset_y = 0.0;

unsigned int program_id = 0;

unsigned int shader_from_file (std::string path, GLenum shader_type)
{
	std::ifstream is(path);
	if (!is.good())
		return 0;

	std::string source;
	std::string s;

	while (true) {
		std::getline(is, s);
		if (!is.good())
			break;
		s += '\n';
		source += s;
	}

	unsigned int id = glCreateShader(shader_type);

	const char* src_ptr = source.c_str();

	glShaderSource(id, 1, &src_ptr, nullptr);
	glCompileShader(id);

	int success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success) {
		std::cout << "compiled shader " << path << std::endl;
		return id;
	} else {
		std::cout << "failed to compile shader " << path << ":\n";

		int log_len = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);

		char* log = (char*) malloc(log_len + 1);
		glGetShaderInfoLog(id, log_len, 0, log);

		std::cout << log << std::endl;

		return 0;
	}
}


bool init ()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	sdlwindow = SDL_CreateWindow("opengl window",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			resolution_x, resolution_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (sdlwindow == nullptr)
		return false;

	sdlcontext = SDL_GL_CreateContext(sdlwindow);

	if (sdlcontext == nullptr)
		return false;

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return false;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	return true;
}

void render ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glFrustum(-1, 1, -1, 1, 0.1, 100);
	glTranslatef(offset_x, offset_y, 0.0);
	glRotatef(10.0f, 0.0, offset_x, offset_y);

	glBegin(GL_TRIANGLE_FAN);

	glVertex3f(0, 0, -0.4);

	float x = -0.5;
	float y = -0.5;

	int w = 4;
	while (w--) {
		glVertex3f(x, y, -0.2);
		std::swap(x, y);
		y = -y;
	}

	glEnd();

	glPopMatrix();
}

int main ()
{
	if (!init()) {
		std::cout << "Failed init\n";
		return 1;
	}

	SDL_Event ev;
	bool quit = false;

	unsigned int frag_shader = shader_from_file("bin/demo_fragment.glsl", GL_FRAGMENT_SHADER);
	unsigned int vert_shader = shader_from_file("bin/demo_vertex.glsl", GL_VERTEX_SHADER);

	program_id = glCreateProgram();

	glAttachShader(program_id, frag_shader);
	glAttachShader(program_id, vert_shader);
	glLinkProgram(program_id);
	glUseProgram(program_id);

	SDL_StartTextInput();

	while (!quit) {

		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:

				quit = true;
				break;

			case SDL_TEXTINPUT:

				switch (ev.text.text[0]) {
				case 'q':
					quit = true;
					break;
				}
				break;
			}
		}

		int x, y;
		SDL_GetMouseState(&x, &y);

		x *= 2;
		y *= 2;
		x -= resolution_x;
		y -= resolution_y;

		offset_x =  (float) x / resolution_x;
		offset_y = -(float) y / resolution_y;

		render();

		SDL_GL_SwapWindow(sdlwindow);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
			std::cout << "GL ERROR " << err << std::endl;
	}

	SDL_StopTextInput();

	return 0;
}
