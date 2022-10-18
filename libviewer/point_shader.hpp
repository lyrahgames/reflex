#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

inline auto point_shader() -> shader_program {
  constexpr czstring vertex_shader_text =
      "#version 330 core\n"

      "struct Camera {"
      "  mat4 projection;"
      "  mat4 view;"
      "  mat4 viewport;"
      "};"

      "uniform Camera camera;"

      "layout (location = 0) in vec3 p;"

      "void main(){"
      "  gl_Position = camera.projection * camera.view * vec4(p, 1.0);"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "layout (location = 0) out vec4 frag_color;"

      "void main(){"
      "  frag_color = vec4(0.8, 0.5, 0.0, 1.0);"
      "}";

  return shader_program{vertex_shader_text, fragment_shader_text};
}

}  // namespace viewer
