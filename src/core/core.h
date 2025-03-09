#pragma once
#include "misc.h"
#include <string>

extern bool must_quit;
extern int exit_code;
extern unsigned long long tick;

void update();

// Entity format:
// entity-class
//   key1 value1
//   key2 value2
//   ! event-name delay target signal...
// etc.
// The line with the entity class must not start with whitespace;
// the lines with key-value pairs must start with whitespace.
// Those lines among key-value pairs which start with !
//   are event handlers, which tell the entity to send a signal
//   when something happens.

void load_map(std::string path);
void init_core();

// General purpose

// Axially-aligned bounding box
struct Bbox {
  vec3 start;
  vec3 end;

  void expand(const vec3& pt);
  void expand(const Bbox& other);

  bool point_in(const vec3& pt) const;
  bool point_in(const vec3& pt, float tolerance) const;
  float volume() const;

  bool intersects(const Bbox& b) const;

  // In a case with no intersection, the _guarded version
  // brings the box to a state where its volume is 0.
  // The regular version might end up with a box which has
  // exactly two negative sides, giving volume > 0
  void intersect(const Bbox& b);
  void intersect_guarded(const Bbox& b);

  inline const float* data() const {
    return glm::value_ptr(start);
  }
};

static_assert(
  offsetof(Bbox, start) == 0 && offsetof(Bbox, end) == sizeof(vec3)
    && sizeof(vec3) == 3 * sizeof(float),
  "Alignment for Bbox is broken: data() will not work"
);