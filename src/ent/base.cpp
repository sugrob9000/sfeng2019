#include "base.h"
#include "core/signal.h"
#include "render/vis.h"
#include <sstream>

// IWYU pragma: begin_keep, we include an Xmacro file later that uses these
#include "ent/lights.h"
#include "ent/prop.h"
#include "ent/timer.h"
#include "ent/trigger_sphere.h"
// IWYU pragma: end_keep

EntRegistry ent_reg;

void BaseEntity::set_name(const std::string& new_name) {
  name = new_name;
  BaseEntity* another = ents.find_by_name(new_name);
  if (another != nullptr) {
    warning("Entity at %p stole name %s from entity at %p", this, name.c_str(), another);
    another->set_name("");
  }
  ents.name_index[name] = this;
}

void BaseEntity::on_event(const std::string& event) const {
  auto i = events.find(event);
  if (i == events.end())
    return;
  for (const Signal& s: i->second)
    add_signal(s);
}

void BaseEntity::moved() {
  vis_requery_entity(this);
}

void BaseEntity::apply_keyvals(const EntKeyvals& kv) {
  KV_TRY_GET(kv["pos"], atovec3(val, pos);, pos = vec3(););
  KV_TRY_GET(kv["ang"], atovec3(val, ang);, ang = vec3(););
  KV_TRY_GET(kv["name"], set_name(val), name = "");
}

void fill_ent_registry() {
#define ENTITY(NAME, CLASS) \
  ent_reg[NAME] = &ent_factory<CLASS>; \
  fill_io_data<CLASS>();
#include "ent/list.inc"
#undef ENTITY
}

WorldEntityList ents;

BaseEntity* WorldEntityList::spawn(std::string type) {
  EntSpawnerFptr spawner = ent_reg[type];
  if (spawner == nullptr)
    return nullptr;
  BaseEntity* ent = spawner();
  vec.push_back(ent);
  return ent;
}

BaseEntity* WorldEntityList::find_by_name(std::string name) {
  auto i = ents.name_index.find(name);
  if (i == ents.name_index.end())
    return nullptr;
  return i->second;
}

/* ================= Key-value maps ================= */

const std::string EntKeyvals::none = "";

const std::string& EntKeyvals::operator[](std::string s) const {
  auto i = m.find(s);
  if (i == m.end())
    return none;
  return i->second;
}

void EntKeyvals::add(std::string key, std::string value) {
  m[key] = value;
}

void EntKeyvals::clear() {
  m.clear();
}

/* ================= Base signals ================= */

template<>
void signal_handler<BaseEntity, SigTag("setpos")>(BaseEntity& e, std::string arg) {
  atovec3(arg, e.pos);
  e.moved();
}

template<>
void signal_handler<BaseEntity, SigTag("addpos")>(BaseEntity& e, std::string arg) {
  e.pos += atovec3(arg);
  e.moved();
}

template<>
void signal_handler<BaseEntity, SigTag("setang")>(BaseEntity& e, std::string arg) {
  atovec3(arg, e.ang);
  e.moved();
}

template<>
void signal_handler<BaseEntity, SigTag("setname")>(BaseEntity& e, std::string arg) {
  e.set_name(arg);
}

template<>
void signal_handler<BaseEntity, SigTag("showpos")>(BaseEntity& e, std::string) {
  std::ostringstream os;
  os << e.name << " - pos " << e.pos << " ang " << e.ang << '\n';
  DEBUG_MSG(os.str());
}
