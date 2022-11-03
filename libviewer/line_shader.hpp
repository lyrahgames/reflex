#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

inline auto line_shader() -> shader_program {
  constexpr czstring vertex_shader_text =
      "#version 330 core\n"

      "struct Camera {"
      "  mat4 projection;"
      "  mat4 view;"
      "  mat4 viewport;"
      "};"

      "uniform Camera camera;"

      "layout (location = 0) in vec3 p;"
      "layout (location = 1) in vec3 n;"
      "layout (location = 2) in float arclength;"
      "layout (location = 3) in float curvature;"

      "out float s;"
      "out float k;"

      "void main(){"
      "  vec4 x = camera.view * vec4(p, 1.0);"
      "  gl_Position = camera.projection * x;"
      "  s = arclength;"
      "  k = curvature;"
      "}";

  constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "layout (location = 0) out vec4 frag_color;"

      "in float s;"
      "in float k;"

      "float colormap_f(float x) {"
      "  return ((-2010.0 * x + 2502.5950459) * x - 481.763180924) / 255.0;"
      "}"

      "float colormap_red(float x) {"
      "    if (x < 0.0) {"
      "        return 3.0 / 255.0;"
      "    } else if (x < 0.238) {"
      "        return ((-1810.0 * x + 414.49) * x + 3.87702) / 255.0;"
      "    } else if (x < 51611.0 / 108060.0) {"
      "        return (344441250.0 / 323659.0 * x - 23422005.0 / 92474.0) / "
      "255.0;"
      "    } else if (x < 25851.0 / 34402.0) {"
      "        return 1.0;"
      "    } else if (x <= 1.0) {"
      "        return (-688.04 * x + 772.02) / 255.0;"
      "    } else {"
      "        return 83.0 / 255.0;"
      "    }"
      "}"

      "float colormap_green(float x) {"
      "    if (x < 0.0) {"
      "        return 0.0;"
      "    } else if (x < 0.238) {"
      "        return 0.0;"
      "    } else if (x < 51611.0 / 108060.0) {"
      "        return colormap_f(x);"
      "    } else if (x < 0.739376978894039) {"
      "        float xx = x - 51611.0 / 108060.0;"
      "        return ((-914.74 * xx - 734.72) * xx + 255.) / 255.0;"
      "    } else {"
      "        return 0.0;"
      "    }"
      "}"

      "float colormap_blue(float x) {"
      "    if (x < 0.0) {"
      "        return 19.0 / 255.0;"
      "    } else if (x < 0.238) {"
      "        float xx = x - 0.238;"
      "        return (((1624.6 * xx + 1191.4) * xx + 1180.2) * xx + 255.0) / "
      "255.0;"
      "    } else if (x < 51611.0 / 108060.0) {"
      "        return 1.0;"
      "    } else if (x < 174.5 / 256.0) {"
      "        return (-951.67322673866 * x + 709.532730938451) / 255.0;"
      "    } else if (x < 0.745745353439206) {"
      "        return (-705.250074130877 * x + 559.620050530617) / 255.0;"
      "    } else if (x <= 1.0) {"
      "        return ((-399.29 * x + 655.71) * x - 233.25) / 255.0;"
      "    } else {"
      "        return 23.0 / 255.0;"
      "    }"
      "}"

      "vec4 colormap(float x) {"
      "    return vec4(colormap_red(x), colormap_green(x), colormap_blue(x), "
      "1.0);"
      "}"

      "void main(){"
      // "  frag_color = vec4(0.8, 0.5, 0.0, 1.0);"
      "  frag_color = colormap(abs(k));"
      "}";

  return shader_program{vertex_shader_text, fragment_shader_text};
}

}  // namespace viewer
