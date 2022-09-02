#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

inline auto contours_shader() -> shader_program {
  constexpr czstring vertex_shader_text =
      "#version 330 core\n"

      "struct Camera {"
      "  mat4 projection;"
      "  mat4 view;"
      "  mat4 viewport;"
      "};"
      "uniform Camera camera;"

      "uniform mat4 model;"

      "layout (location = 0) in vec3 p;"
      "layout (location = 1) in vec3 n;"
      "layout (location = 2) in vec2 uv;"

      "out vec3 position;"
      "out vec3 normal;"
      "out vec2 texuv;"

      "void main(){"
      "  gl_Position = camera.projection * camera.view * model * vec4(p, 1.0);"
      "  position = vec3(camera.view * model * vec4(p, 1.0));"
      "  normal = vec3(camera.view * model * vec4(n, 0.0));"
      "  texuv = uv;"
      "}";

  constexpr czstring geometry_shader_text =
      "#version 330 core\n"

      "layout (triangles) in;"
      "layout (line_strip, max_vertices = 2) out;"

      "in vec3 position[];"
      "in vec3 normal[];"
      "in vec2 texuv[];"

      "void main(){"
      "  vec4 a = gl_in[0].gl_Position;"
      "  vec4 b = gl_in[1].gl_Position;"
      "  vec4 c = gl_in[2].gl_Position;"

      "  float sa = dot(normal[0], position[0]);"
      "  float sb = dot(normal[1], position[1]);"
      "  float sc = dot(normal[2], position[2]);"

      "  if (sa * sb < 0) {"
      "    gl_Position = ((abs(sb) * a + abs(sa) * b) / (abs(sa) + abs(sb)));"
      "    EmitVertex();"
      "  }"
      "  if (sa * sc < 0) {"
      "    gl_Position = ((abs(sc) * a + abs(sa) * c) / (abs(sa) + abs(sc)));"
      "    EmitVertex();"
      "  }"
      "  if (sb * sc < 0) {"
      "    gl_Position = ((abs(sc) * b + abs(sb) * c) / (abs(sb) + abs(sc)));"
      "    EmitVertex();"
      "  }"
      "  EndPrimitive();"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"
      "layout (location = 0) out vec4 frag_color;"
      "void main(){"
      "  frag_color = vec4(vec3(0.0), 1.0);"
      "}";

  vertex_shader vs{vertex_shader_text};
  geometry_shader gs{geometry_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, gs, fs};
}

}  // namespace viewer
