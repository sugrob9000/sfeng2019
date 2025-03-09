#pragma once
#include <algorithm>
#include <map>
#include <queue>
#include <string>

// Entities can be sent signals to. Each entity class can implement
//   handlers for its own signals, and set them up at engine startup.
// For example, a door class may have a handler for
//   "open" and "close" signals.
//
// A signal knows:
//   - its target entity, by name
//   - the tick on which it is due to happen
//   - what signal for the target to execute and with which argument
// Note that it does not know the class of which its target is,
//   so invalid signals should be tolerated.
//
// Name resolution happens *when the delay is up*,
//   not when the signal is sent.

struct Signal {
  std::string target;
  long long tick_due;
  std::string signal_name;
  std::string argument;

  void execute() const;
};


class BaseEntity;
using SigHandlerFptr = void (*)(BaseEntity* ent, std::string arg);
using Sigmap = std::map<std::string, SigHandlerFptr>;

template<class Entity>
Sigmap sigmap;

// Keep a queue of signals, sorted by when they
// are due to happen, ascending
bool operator<(const Signal& a, const Signal& b);
extern std::priority_queue<Signal> signals;

// The basic routine used to fire a signal
void add_signal(Signal s);

// These must be usable at compile time, because they are used as NTTPs
struct SigTag {
  char name[20] = {};
  constexpr explicit SigTag(std::string_view n) {
    std::copy(n.begin(), n.end(), name);
  }
};

template<typename Entity, SigTag Tag>
void signal_handler(Entity& ent, std::string argument);

namespace detail {
  template<typename DestEntity, typename SrcEntity, SigTag Signal>
  void register_signal_handler() {
    ::sigmap<DestEntity>[Signal.name] = (SigHandlerFptr) &::signal_handler<SrcEntity, Signal>;
  }
}

// Called by everyone's `fill_io_data()` instantiations. DO NOT SPECIALIZE.
template<typename Entity, SigTag... AddlSignals>
void do_fill_io_data() {
  // Register the basic signals
  detail::register_signal_handler<Entity, BaseEntity, SigTag("setpos")>();
  detail::register_signal_handler<Entity, BaseEntity, SigTag("addpos")>();
  detail::register_signal_handler<Entity, BaseEntity, SigTag("setang")>();
  detail::register_signal_handler<Entity, BaseEntity, SigTag("setname")>();
  detail::register_signal_handler<Entity, BaseEntity, SigTag("showpos")>();

  // Register custom signals
  (detail::register_signal_handler<Entity, Entity, AddlSignals>(), ...);
}

// An entity may specialize this (IN THE HEADER FILE) to register own signals.
// In the specialization, it must call `do_fill_io_data` with more SigTag templace arguments.
template<class Entity>
void fill_io_data() {
  // By default, provide no additional signals
  do_fill_io_data<Entity>();
}

// Events: each entity object (as opposed to class) may specify
//   on which events it wants to fire certain signals
//   (maybe, several on one event).
// For exmaple, a particular trigger volume may want to tell a
//   particular door to open when someone steps in it.
using EventMap = std::map<std::string, std::vector<Signal>>;