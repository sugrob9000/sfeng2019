cmake_minimum_required(VERSION 3.27)
project(sfeng2019 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(
  engine
  src/core/core.cpp
  src/core/signal.cpp
  src/ent/base.cpp
  src/ent/lights.cpp
  src/ent/prop.cpp
  src/ent/timer.cpp
  src/ent/trigger_sphere.cpp
  src/input/bind.cpp
  src/input/console.cpp
  src/input/input.cpp
  src/main.cpp
  src/misc.cpp
  src/render/ctx.cpp
  src/render/debug.cpp
  src/render/framebuffer.cpp
  src/render/gbuffer.cpp
  src/render/light/all.cpp
  src/render/light/cone.cpp
  src/render/light/sun.cpp
  src/render/material.cpp
  src/render/model.cpp
  src/render/render.cpp
  src/render/resource.cpp
  src/render/sky.cpp
  src/render/vis.cpp
)

target_link_libraries(
  engine PRIVATE
  GL
  GLEW
  SDL2
  SDL2_image
  SDL2_ttf
)
target_include_directories(engine PRIVATE src)

target_compile_options(
  engine PRIVATE
  -Wall
  -Wextra
  -Wno-sign-compare
  -Wno-unused
  -Wno-attributes
  -Wno-cast-function-type
  -Wshadow
)

set_target_properties(engine PROPERTIES UNITY_BUILD ON)