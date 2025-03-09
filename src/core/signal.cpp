#include "signal.h"
#include "core.h"
#include "ent/base.h"
#include "input/cmds.h"

std::priority_queue<Signal> signals;

bool operator<(const Signal& a, const Signal& b) {
  return a.tick_due > b.tick_due;
}

void Signal::execute() const {
  BaseEntity* e = ents.find_by_name(target);
  if (e == nullptr) {
    warning("tried to send signal to nonexistent entity %s", target.c_str());
    return;
  }

  const Sigmap& sigmap = e->get_sigmap();
  auto i = sigmap.find(signal_name);

  if (i == sigmap.end())
    return;

  SigHandlerFptr routine = i->second;
  if (routine != nullptr)
    routine(e, argument);
}

void add_signal(Signal s) {
  s.tick_due += tick;
  signals.push(s);
}

COMMAND_ROUTINE(signal) {
  if (ev != PRESS)
    return;

  if (args.size() < 3)
    return;

  int delay = atoi(args[0].c_str());
  const std::string& recipient = args[1];
  const std::string& signame = args[2];

  std::string sigarg;
  if (args.size() >= 4) {
    for (unsigned int i = 3; i < args.size(); i++) {
      sigarg += args[i];
      sigarg += " ";
    }
    sigarg.pop_back();
  }
  Signal s = {recipient, delay, signame, sigarg};
  add_signal(s);
}