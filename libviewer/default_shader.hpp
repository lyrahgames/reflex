#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

inline auto default_shader() -> shader_program {
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

      // "out vec3 normal;"
      // "out vec2 texuv;"

      "out vertex_data {"
      "  vec3 normal;"
      "  vec2 texuv;"
      "} v;"

      "void main(){"
      "  gl_Position = camera.projection * camera.view * model * vec4(p, 1.0);"
      "  v.normal = vec3(camera.view * model * vec4(n, 0.0));"
      "  v.texuv = uv;"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "struct Material {"
      "  vec3 ambient;"
      "  vec3 diffuse;"
      "  vec3 specular;"
      "  float shininess;"
      "  sampler2D texture;"
      "};"
      "uniform Material material;"

      // "in vec3 normal;"
      // "in vec2 texuv;"

      "in vertex_data {"
      "  vec3 normal;"
      "  vec2 texuv;"
      "} v;"

      "layout (location = 0) out vec4 frag_color;"

      "void main(){"
      "  vec3 n = normalize(v.normal);"

      "  vec3 light_color = vec3(0.3, 0.3, 0.3);"

      "  vec3 view_dir = vec3(0.0, 0.0, 1.0);"
      "  vec3 light_dir = view_dir;"
      "  vec3 reflect_dir = reflect(-light_dir, n);"

      "  vec3 color = material.ambient;"

      "  float d = max(dot(light_dir, n), 0.0);"
      "  color += d * material.diffuse;"

      "  float s = pow(max(dot(reflect_dir, n), 0.0), material.shininess);"
      "  color += s * material.specular;"

      "  vec3 tex = vec3(texture(material.texture, v.texuv));"
      "  color *= tex * light_color;"

      "  frag_color = vec4(color, 1.0);"

      "}";

  return shader_program{vertex_shader_text, fragment_shader_text};
}

}  // namespace viewer
