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
      "  vec4 x = camera.view * vec4(p, 1.0);"
      "  gl_Position = camera.projection * x;"
      "  gl_PointSize = 10;"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "layout (location = 0) out vec4 frag_color;"

      // "void main(){"
      // "  frag_color = vec4(0.8, 0.5, 0.0, 1.0);"
      // "}"

      "void main(){"
      "  float alpha = 1.0;"
      "  vec2 tmp = 2.0 * gl_PointCoord - 1.0;"
      // "  if (dot(tmp, tmp) > 1.0) alpha = 0.0;"
      // "  if ((abs(tmp.x) + abs(tmp.y)) > 1.0) discard;"
      // "  float r = dot(tmp, tmp);"
      // "  float delta = fwidth(0.5 * r);"
      "  float delta = 0.2;"
      "  float p = 2.0 / 3.0;"
      "  float r = pow(pow(abs(tmp.x), p) + pow(abs(tmp.y), p), 1.0 / p);"
      // "  float delta = fwidth(0.5 * r * r);"
      "  alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r);"
      "  frag_color = vec4(0.1, 0.3, 0.7, alpha);"
      "}";

  return shader_program{vertex_shader_text, fragment_shader_text};
}

}  // namespace viewer
