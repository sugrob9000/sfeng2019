#pragma once
#include "core/core.h"
#include "core/signal.h"
#include "misc.h"
#include <map>

// Key-value pairs for entities

struct EntKeyvals {
  std::map<std::string, std::string> m;
  static const std::string none;

  const std::string& operator[](std::string s) const;
  void add(std::string key, std::string value);
  void clear();
};

// Try to get a value from an EntKeyvals,
// run code in the parameter if_there if it's there, or
// run code in the parameter if_not_there if it's not
// Example:
//
// const EntKeyvals& kv = ...;
// KV_TRY_GET(kv["pos"],
//    atovec3(val, pos); ,
//    pos = vec3(0.0, 0.0, 0.0); );
#define KV_TRY_GET(kvv, if_there, if_not_there) \
  do { \
    const std::string& val = kvv; \
    if (val.empty()) { \
      if_not_there; \
    } else { \
      if_there; \
    } \
  } while (false)

// The base entity class
class BaseEntity {
public:
  vec3 pos;
  vec3 ang;
  std::string name;

  virtual void think() {}
  // By default, will read pos, ang, and name.
  // Every entity should probably still call this.
  virtual void apply_keyvals(const EntKeyvals& kv);

  EventMap events;

  // We have to be able to get the sigmap knowing only the pointer
  // to the entity, while in runtime - templates won't help with this
  // The implementation is inserted by the preprocessor
  virtual const Sigmap& get_sigmap() const = 0;

  void on_event(const std::string& event) const;
  void set_name(const std::string& name);

  virtual void render() const {}

  // The entity promises that it is fully inside the box returned
  // Entities that have no physical appearance (ie logical ones)
  //   may express this by returning a box with volume 0
  virtual Bbox get_bbox() const { return {}; }

  // Updates the engine's idea of where the entity is, for
  // purposes such as vis. Call this on an entity whenever its
  // output of get_bbox() changes
  virtual void moved();

  // Used in vis to avoid redundant rendering
  uint64_t render_last_guard_key;
};

// Inserted into the entity class definition
#define ENT_IMPLEMENT_GET_SIGMAP(NAME) \
  const Sigmap& get_sigmap() const override { return sigmap<NAME>; }

// Mapping entity class names (such as prop)
// to C++ classes
typedef BaseEntity* (*EntSpawnerFptr)();
typedef std::map<std::string, EntSpawnerFptr> EntRegistry;
extern EntRegistry ent_reg;
void fill_ent_registry();

template<class e_derived>
BaseEntity* ent_factory() {
  return new e_derived;
}

// A world's currently existing entities
struct WorldEntityList {
  std::vector<BaseEntity*> vec;
  std::map<std::string, BaseEntity*> name_index;
  BaseEntity* spawn(std::string type);
  BaseEntity* find_by_name(std::string name);
};

extern WorldEntityList ents;