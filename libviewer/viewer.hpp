#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

class viewer {
 public:
  viewer(int w, int h);
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
