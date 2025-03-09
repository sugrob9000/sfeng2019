#pragma once
#include <SDL2/SDL_events.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

const uint8_t PRESS = 0;
const uint8_t RELEASE = 1;

typedef std::vector<std::string> CmdArgs;

typedef void (*CmdRoutineFptr)(const CmdArgs&, uint8_t);
typedef void (*MousemoveFptr)(int dx, int dy, int abx, int aby);

// Instances of this class are *not* supposed to be created
// a lot during runtime. The key binding maps holds instances of them,
// which are created once during engine start.
// They are also created when parsing a console command, which is
// not often either.
struct Command {
  std::string name;
  CmdArgs args;
};

struct CommandRegistry {
  std::unordered_map<std::string, CmdRoutineFptr> m;

  void register_command(std::string name, CmdRoutineFptr routine);
  void run(const Command& cmd, uint8_t ev);
};

extern CommandRegistry cmd_registry;
extern MousemoveFptr mousemove_proc;

void init_input();
void run_argv_commands(int argc, const char* const* argv);
void handle_input();

// Supports prepending + or - for PRESS and RELEASE, respectively
// (+ is unnecessary, since PRESS is the default anyway)
void run_cmd_ext(const std::string& cmd);
void run_script(std::string path);

Command parse_command(const char* cmd);

namespace console {
// A console window for entering commands
// Enter - execute
// Esc - get out
// Shift-backspace - erase current command
void handle_ev(const SDL_Event& e);
void open_console();
void close_console();
void render_console();
extern bool console_active;
} // namespace console