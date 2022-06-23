#include <libviewer/viewer.hpp>

namespace viewer {

namespace {

// Structure and vertex data for the triangle to be rendered.
const struct {
  float x, y;     // 2D Position
  float r, g, b;  // Color
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

const char* vertex_shader_text =
    "#version 330 core\n"
    "uniform mat4 MVP;"
    "attribute vec3 vCol;"
    "attribute vec2 vPos;"
    "varying vec3 color;"
    "void main(){"
    "  gl_Position = MVP * vec4(vPos, 0.0, 1.0);"
    "  color = vCol;"
    "}";
const char* fragment_shader_text =
    "#version 330 core\n"
    "varying vec3 color;"
    "void main(){"
    "  gl_FragColor = vec4(color, 1.0);"
    "}";

}  // namespace

viewer::viewer(int w, int h) : screen_width(w), screen_height(h) {
  // The shader has to be initialized before
  // the initialization of the vertex data
  // due to identifier location variables
  // that have to be set after creating the shader program.
  init_shader();
  init_vertex_data();
  // To initialize the viewport and matrices,
  // window has to be resized at least once.
  resize();
}

viewer::~viewer() {
  // Delete vertex data.
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array);
  // Delete shader program.
  glDeleteProgram(program);
}

void viewer::init_shader() {
  // Compile and create the vertex shader.
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
  glCompileShader(vertex_shader);
  {
    // Check for errors.
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to compile vertex shader!: ") +
          info_log);
    }
  }

  // Compile and create the fragment shader.
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
  glCompileShader(fragment_shader);
  {
    // Check for errors.
    GLint success;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to compile fragment shader!: ") +
          info_log);
    }
  }

  // Link vertex shader and fragment shader to shader program.
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  {
    // Check for errors.
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetProgramInfoLog(program, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to link shader program!: ") + info_log);
    }
  }

  // Delete unused shaders.
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Get identifier locations in the shader program
  // to change their values from the outside.
  mvp_location = glGetUniformLocation(program, "MVP");
  vpos_location = glGetAttribLocation(program, "vPos");
  vcol_location = glGetAttribLocation(program, "vCol");
}

void viewer::init_vertex_data() {
  // Use a vertex array to be able to reference the vertex buffer and
  // the vertex attribute arrays of the triangle with one single variable.
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  // Generate and bind the buffer which shall contain the triangle data.
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set the data layout of the position and colors
  // with vertex attribute pointers.
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)(sizeof(float) * 2));
}

void viewer::resize() {
  const auto aspect_ratio = float(screen_width) / screen_height;
  // Make sure rendering takes place in the full screen.
  glViewport(0, 0, screen_width, screen_height);
  // Use a perspective projection with correct aspect ratio.
  projection = glm::perspective(45.0f, aspect_ratio, 0.1f, 100.f);
  // Position the camera in space by using a view matrix.
  view = translate(mat4(1.0f), vec3(0.0f, 0.0f, -2));
}

void viewer::resize(int width, int height) {
  screen_width = width;
  screen_height = height;
  resize();
}

void viewer::update() {
  const auto new_time = clock::now();
  const auto t = duration<float>(new_time - time).count();

  // Compute and set MVP matrix in shader.
  model = glm::mat4{1.0f};
  const auto axis = glm::normalize(glm::vec3(1, 1, 1));
  model = rotate(model, t, axis);
  const auto mvp = projection * view * model;
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
}

void viewer::render() {
  // Clear the screen.
  glClear(GL_COLOR_BUFFER_BIT);

  // Bind vertex array of triangle
  // and use the created shader
  // to render the triangle.
  glUseProgram(program);
  glBindVertexArray(vertex_array);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

}  // namespace viewer
