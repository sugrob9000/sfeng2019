#include "base.h"
#include "core/signal.h"
#include "misc.h"
#include "render/vis.h"
#include <sstream>

// IWYU pragma: begin_keep, we include an Xmacro file later that uses these
#include "ent/lights.h"
#include "ent/prop.h"
#include "ent/timer.h"
#include "ent/trigger_sphere.h"
// IWYU pragma: end_keep

EntClassRegistry global_ent_class_registry;

void BaseEntity::set_name(const std::string& new_name) {
  name = new_name;
  BaseEntity* other = global_entity_list.find_by_name(new_name);
  if (other != nullptr) {
    warning("Entity at %p stole name %s from entity at %p", this, name.c_str(), other);
    other->set_name("");
  }
  global_entity_list.name_index[name] = this;
}

void BaseEntity::on_event(const std::string& event) const {
  auto i = events.find(event);
  if (i == events.end())
    return;
  for (const Signal& s: i->second)
    add_signal(s);
}

void BaseEntity::on_moved() {
  vis_requery_entity(this);
}

void BaseEntity::move_to(vec3 newpos) {
  pos = newpos;
  on_moved();
}

void BaseEntity::rotate_to(vec3 newang) {
  ang = newang;
  on_moved();
}

void BaseEntity::render(uint64_t sentinel) const {
  if (last_render_sentinel != sentinel) {
    last_render_sentinel = sentinel;
    do_render();
  }
}

void BaseEntity::register_event(std::string event_name, Signal signal) {
  events[event_name].push_back(std::move(signal));
}

void BaseEntity::apply_keyvals(const EntKeyvals& kv) {
  pos = kv.transform_with_default("pos", stovec3, vec3());
  ang = kv.transform_with_default("ang", stovec3, vec3());
  set_name(kv.get_with_default("name", ""));
}

void fill_ent_class_registry() {
  #define ENTITY(NAME, CLASS) \
    global_ent_class_registry[NAME] = &(ent_factory<CLASS>); \
    fill_io_data<CLASS>();
  #include "ent/list.inc"
  #undef ENTITY
}

WorldEntityList global_entity_list;

BaseEntity* WorldEntityList::spawn(std::string type) {
  SpawnEntityFptr spawner = global_ent_class_registry[type];
  if (spawner == nullptr)
    return nullptr;
  vec.push_back(spawner());
  return vec.back().get();
}

BaseEntity* WorldEntityList::find_by_name(std::string name) {
  auto i = global_entity_list.name_index.find(name);
  if (i == global_entity_list.name_index.end())
    return nullptr;
  return i->second;
}

// ================= Key-value maps =================

void EntKeyvals::add(std::string key, std::string value) {
  m[key] = value;
}

void EntKeyvals::clear() {
  m.clear();
}

const std::string* EntKeyvals::get(std::string key) const {
  auto it = m.find(key);
  if (it == m.end()) {
    return nullptr;
  }
  return &it->second;
}

std::string EntKeyvals::get_with_default(std::string key, std::string def) const {
  auto* found = get(key);
  return found ? *found : def;
}

// ================= Base signals =================

template<>
void signal_handler<BaseEntity, SigTag("setpos")>(BaseEntity& e, std::string arg) {
  e.move_to(atovec3(arg));
}

template<>
void signal_handler<BaseEntity, SigTag("addpos")>(BaseEntity& e, std::string arg) {
  e.move_by(atovec3(arg));
}

template<>
void signal_handler<BaseEntity, SigTag("setang")>(BaseEntity& e, std::string arg) {
  e.rotate_to(atovec3(arg));
}

template<>
void signal_handler<BaseEntity, SigTag("setname")>(BaseEntity& e, std::string arg) {
  e.set_name(arg);
}

template<>
void signal_handler<BaseEntity, SigTag("showpos")>(BaseEntity& e, std::string) {
  std::ostringstream os;
  os << e.get_name() << " - pos " << e.get_pos() << " ang " << e.get_ang() << '\n';
  DEBUG_MSG(os.str());
}
