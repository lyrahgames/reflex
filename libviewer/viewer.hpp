#pragma once
#include <libviewer/scene.hpp>
#include <libviewer/utility.hpp>
//
#include <libviewer/contours_shader.hpp>
#include <libviewer/default_shader.hpp>
#include <libviewer/wireframe_shader.hpp>

namespace viewer {

class viewer {
 public:
  static constexpr const char* title = "OpenGL Basic Framework";
  static constexpr int initial_screen_width = 800;
  static constexpr int initial_screen_height = 450;

  static constexpr size_t context_version_major = 3;
  static constexpr size_t context_version_minor = 3;

  static constexpr czstring glsl_version_macro_code = "#version 330 core\n";

  viewer(int w = initial_screen_width, int h = initial_screen_height);
  ~viewer();

  void resize(int width, int height);
  void resize();
  void update();
  void render();

  void update_view();
  void turn(const vec2& angle);
  void shift(const vec2& pixels);
  void zoom(float scale);
  void set_y_as_up();
  void set_z_as_up();

  void fit_view();
  void load_model(czstring file_path);

  void load_shader(czstring path);

 private:
  int screen_width, screen_height;
  time_type time = clock::now();

  shader_program shader = default_shader();

  // World Origin
  vec3 origin;
  // Basis Vectors of Right-Handed Coordinate System
  vec3 up{0, 1, 0};
  vec3 right{1, 0, 0};
  vec3 front{0, 0, 1};
  // Spherical/Horizontal Coordinates of Camera
  float radius = 10;
  float altitude = 0;
  float azimuth = 0;

  bool view_should_update = true;

  camera cam{};

  struct scene scene;

  vec3 aabb_min{};
  vec3 aabb_max{};
  float bounding_radius;
};

}  // namespace viewer
