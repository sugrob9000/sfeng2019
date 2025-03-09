#pragma once
#include "core/core.h"
#include "core/signal.h"
#include "misc.h"
#include <map>
#include <memory>

// Key-value pairs for entities

struct EntKeyvals {
  std::map<std::string, std::string> m;

  void add(std::string key, std::string value);
  void clear();
  const std::string* get(std::string) const;
  std::string get_with_default(std::string key, std::string d) const;

  // no monadic interface? :(
  template<typename T>
  T transform_with_default(std::string key, auto f, T def) const {
    if (auto* value = get(std::move(key))) {
      return f(*value);
    } else {
      return def;
    }
  }
};

// The base entity class
class BaseEntity {
public:
  BaseEntity() = default;
  virtual ~BaseEntity() = default;
  virtual void think() {}

  // We have to be able to get the sigmap knowing only the pointer
  // to the entity, while in runtime - templates won't help with this
  // The implementation is inserted by the preprocessor
  virtual const Sigmap& get_sigmap() const = 0;

  // Must be called with appropriate global rendering context setup
  // (RenderContext, stage, OpenGL state... messy)
  // Sentinel is basically the ID of the batch in which it is being rendered.
  // Calling render() again with the same sentinel as last time has no effect.
  void render(uint64_t sentinel) const;

  // The entity promises that it is fully inside the box returned
  // Entities that have no physical appearance (i.e. logic-only entities)
  // may return a box with volume 0
  virtual Bbox get_bbox() const { return {}; }

  // By default, will read pos, ang, and name.
  // Every entity should probably still call this.
  virtual void apply_keyvals(const EntKeyvals& kv);

  vec3 get_pos() const { return pos; }
  vec3 get_ang() const { return ang; }

  const std::string& get_name() { return name; }
  void set_name(const std::string& name);

  void move_to(vec3);
  void move_by(vec3 offset) { move_to(pos + offset); }

  void rotate_to(vec3);
  void rotate_by(vec3 offset) { rotate_to(ang + offset); }

  void on_event(const std::string& event) const;

  // Updates the engine's idea of where the entity is, for
  // purposes such as vis. Call this on an entity whenever its
  // output of get_bbox() changes
  virtual void on_moved();

  void register_event(std::string name, Signal);

protected:
  virtual void do_render() const {}

private:
  EventMap events;
  vec3 pos{};
  vec3 ang{};
  std::string name;

  // Used in vis to avoid redundant rendering
  mutable uint64_t last_render_sentinel;
};

// Inserted into the entity class definition
#define ENT_IMPLEMENT_GET_SIGMAP(NAME) \
  const Sigmap& get_sigmap() const override { return sigmap<NAME>; }

// Mapping entity class names (such as prop)
// to C++ classes
using SpawnEntityFptr = std::unique_ptr<BaseEntity> (*)();
using EntClassRegistry = std::map<std::string, SpawnEntityFptr>;
extern EntClassRegistry global_ent_class_registry;
void fill_ent_class_registry();

template<class Derived>
std::unique_ptr<BaseEntity> ent_factory() {
  return std::make_unique<Derived>();
}

// A world's currently existing entities
struct WorldEntityList {
  std::vector<std::unique_ptr<BaseEntity>> vec;
  std::map<std::string, BaseEntity*> name_index;
  BaseEntity* spawn(std::string type);
  BaseEntity* find_by_name(std::string name);
};
extern WorldEntityList global_entity_list;