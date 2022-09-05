#pragma once
#include <opengl/utility.hpp>

namespace opengl {

struct shader_link_error : runtime_error {
  using base = runtime_error;
  shader_link_error(auto&& x) : base(std::forward<decltype(x)>(x)) {}
};

class shader_program {
 public:
  shader_program() = default;

  shader_program(const vertex_shader& vs,
                 const geometry_shader& gs,
                 const fragment_shader& fs,
                 auto&& warning_handle) {
    receive_handle();
    glAttachShader(handle, vs);
    glAttachShader(handle, gs);
    glAttachShader(handle, fs);
    link(std::forward<decltype(warning_handle)>(warning_handle));
  }
  shader_program(const vertex_shader& vs,
                 const geometry_shader& gs,
                 const fragment_shader& fs)
      : shader_program{vs, gs, fs, warnings_as_errors} {}

  shader_program(const vertex_shader& vs,
                 const fragment_shader& fs,
                 auto&& warning_handle) {
    receive_handle();
    glAttachShader(handle, vs);
    glAttachShader(handle, fs);
    link(std::forward<decltype(warning_handle)>(warning_handle));
  }
  shader_program(const vertex_shader& vs, const fragment_shader& fs)
      : shader_program{vs, fs, warnings_as_errors} {}

  ~shader_program() {
    // Zero values are ignored by this function.
    glDeleteProgram(handle);
  }

  // Copying is not allowed.
  shader_program(const shader_program&) = delete;
  shader_program& operator=(const shader_program&) = delete;

  // Moving
  shader_program(shader_program&& x) : handle{x.handle} { x.handle = 0; }
  shader_program& operator=(shader_program&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

  void bind() const { glUseProgram(handle); }

  bool exists() const { return glIsProgram(handle) == GL_TRUE; }

  auto set(czstring name, float value) -> shader_program& {
    glUniform1f(glGetUniformLocation(handle, name), value);
    return *this;
  }

  auto set(czstring name, mat3 data) -> shader_program& {
    glUniformMatrix3fv(glGetUniformLocation(handle, name), 1, GL_FALSE,
                       value_ptr(data));
    return *this;
  }

  auto set(czstring name, mat4 data) -> shader_program& {
    glUniformMatrix4fv(glGetUniformLocation(handle, name), 1, GL_FALSE,
                       value_ptr(data));
    return *this;
  }
  auto set(czstring name, vec3 data) -> shader_program& {
    glUniform3fv(glGetUniformLocation(handle, name), 1, value_ptr(data));
    return *this;
  }
  auto set(czstring name, vec4 data) -> shader_program& {
    glUniform4fv(glGetUniformLocation(handle, name), 1, value_ptr(data));
    return *this;
  }

 private:
  void receive_handle() {
    handle = glCreateProgram();
    if (!handle)
      throw runtime_error(string("Failed to receive shader program handle."));
  }

  bool link_failed() noexcept {
    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    return status == GL_FALSE;
  }

  void write_link_info_log(string& info_log) {
    GLint info_log_size;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_size);
    info_log.resize(info_log_size);
    if (info_log_size)
      glGetProgramInfoLog(handle, info_log_size, nullptr, info_log.data());
  }

  void throw_link_error() {
    throw shader_link_error("Failed to link shader program.");
  }

  void throw_link_error(const string& info_log) {
    throw shader_link_error("Failed to link shader program.\n" + info_log);
  }

  void link() { glLinkProgram(handle); }

  void link(string& info_log) {
    link();
    write_link_info_log(info_log);
    if (link_failed()) throw_link_error(info_log);
  }

  void link(warnings_as_errors_t) {
    link();
    string info_log;
    write_link_info_log(info_log);
    if (link_failed() || !info_log.empty()) throw_link_error(info_log);
  }

  void link(ignore_warnings_t) {
    link();
    if (link_failed()) {
      string info_log{};
      write_link_info_log(info_log);
      throw_link_error(info_log);
    }
  }

  void link(auto&& warning_callback) {
    string info_log;
    link(info_log);
    if (!info_log.empty())
      std::forward<decltype(warning_callback)>(warning_callback)(info_log);
  }

  GLuint handle{};
};

}  // namespace opengl
