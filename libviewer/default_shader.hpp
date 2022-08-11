#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

inline auto default_shader() -> shader_program {
  constexpr czstring vertex_shader_text =
      "#version 330 core\n"

      "uniform mat4 projection;"
      "uniform mat4 view;"
      "uniform mat4 model;"

      "layout (location = 0) in vec3 p;"
      "layout (location = 1) in vec3 n;"
      "layout (location = 2) in vec2 uv;"

      "out vec3 normal;"
      "out vec2 texuv;"

      "void main(){"
      "  gl_Position = projection * view * model * vec4(p, 1.0);"
      "  normal = vec3(model * view * vec4(n, 0.0));"
      "  texuv = uv;"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "uniform sampler2D diffuse_texture;"

      "in vec3 normal;"
      "in vec2 texuv;"
      "layout (location = 0) out vec4 frag_color;"

      "void main(){"
      // "  vec3 tex = vec3(0.25 * texuv, 0.0);"
      "  vec4 tex = texture(diffuse_texture, texuv);"
      "  float ambient = 0.25;"
      "  float diffuse = 0.5 * abs(normalize(normal).z);"
      "  frag_color = vec4(vec3(diffuse + ambient), 1.0);"
      "  frag_color *= tex;"
      "}";

  return shader_program{vertex_shader_text, fragment_shader_text};
}

}  // namespace viewer
