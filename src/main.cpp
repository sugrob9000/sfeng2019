#include "inc_general.h"
#include "render.h"
#include "core/core.h"
#include "core/input.h"

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");

	return 0;
}

