#pragma once
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

 private:
  int screen_width, screen_height;
  time_type time = clock::now();
  // Vertex Data Handles
  GLuint vertex_array;
  GLuint vertex_buffer;
  // Shader Handles
  GLuint program;
  GLint mvp_location, vpos_location, vcol_location;
  // Transformation Matrices
  mat4 model, view, projection;
};

}  // namespace viewer
