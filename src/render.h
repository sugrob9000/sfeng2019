#ifndef RENDER_H
#define RENDER_H

namespace render
{

extern SDL_Window* window;
extern SDL_GLContext context;

extern int resolution_x;
extern int resolution_y;

bool init (int resx, int resy);

}

#endif // RENDER_H

