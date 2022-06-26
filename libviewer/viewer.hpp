#pragma once
#include <libviewer/camera.hpp>
#include <libviewer/shader.hpp>
#include <libviewer/utility.hpp>

namespace viewer {

class viewer {
 public:
  static constexpr const char* title = "OpenGL Basic Framework";
  static constexpr int initial_screen_width = 500;
  static constexpr int initial_screen_height = 500;

  static constexpr size_t context_version_major = 3;
  static constexpr size_t context_version_minor = 3;

  viewer(int w = initial_screen_width, int h = initial_screen_height);
  ~viewer();

  void init_shader();
  void init_vertex_data();
  void resize(int width, int height);
  void resize();
  void update();
  void render();

  void update_view();
  void turn(const vec2& mouse_move);
  void shift(const vec2& mouse_move);
  void zoom(float scale);
  void set_y_as_up();
  void set_z_as_up();

 private:
  int screen_width, screen_height;
  time_type time = clock::now();
  // Vertex Data Handles
  GLuint vertex_array;
  GLuint vertex_buffer;
  // Shader Handles
  shader_program program;
  GLint mvp_location, vpos_location, vcol_location;
  // Transformation Matrices
  mat4 model, view, projection;

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

  // Mouse Interaction
  vec2 old_mouse_pos;
  vec2 mouse_pos;
  bool view_should_update = true;

  camera cam{};
};

}  // namespace viewer
