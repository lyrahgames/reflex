#include <libviewer/default_shader.hpp>

namespace viewer {

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"
    "uniform mat4 model;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"

    "out vec3 normal;"

    "void main(){"
    "  gl_Position = projection * view * model * vec4(p, 1.0);"
    "  normal = vec3(model * view * vec4(n, 0.0));"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "in vec3 normal;"
    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  float ambient = 0.5;"
    "  float diffuse = 0.5 * abs(normalize(normal).z);"
    "  frag_color = vec4(vec3(diffuse + ambient), 1.0);"
    "}";

}  // namespace

auto default_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}

}  // namespace viewer
