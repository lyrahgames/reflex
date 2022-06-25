#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

constexpr auto shader_object_type_name(GLenum shader_object_type) -> czstring {
  switch (shader_object_type) {
    case GL_VERTEX_SHADER:
      return "vertex";
      break;

    case GL_GEOMETRY_SHADER:
      return "geometry";
      break;

    case GL_FRAGMENT_SHADER:
      return "fragment";
      break;

    default:
      return "unknown";
  }
}

constexpr struct warnings_as_errors_t {
} warnings_as_errors{};
constexpr struct ignore_warnings_t {
} ignore_warnings{};

template <GLenum shader_object_type>
class shader_object {
  using string = std::string;

 public:
  struct compile_error : runtime_error {
    using base = runtime_error;
    compile_error(auto&& x) : base(forward<decltype(x)>(x)) {}
  };

  static constexpr auto type() -> GLenum { return shader_object_type; }
  static constexpr auto type_name() -> czstring {
    return shader_object_type_name(type());
  }

  shader_object() = default;

  // Using exceptions for warnings is a bad idea,
  // because they break execution order.

  // Create the shader object by compiling the source code
  // and then writing its info log into the given string reference.

  shader_object(czstring source, auto&& warning_handle) {
    receive_handle();
    compile(source, std::forward<decltype(warning_handle)>(warning_handle));
  }

  shader_object(czstring source) : shader_object{source, warnings_as_errors} {}

  ~shader_object() {
    // Zero values are ignored by this function.
    glDeleteShader(handle);
  }

  // Copying is not allowed.
  shader_object(const shader_object&) = delete;
  shader_object& operator=(const shader_object&) = delete;

  // Moving
  shader_object(shader_object&& x) : handle{x.handle} { x.handle = 0; }
  shader_object& operator=(shader_object&& x) {
    swap(handle, x.handle);
    return *this;
  }

  // This class is only a wrapper.
  // Because of that, the underlying handle can also
  // be used directly and implicitly.
  // This may introduce consistency problems.
  operator GLuint() const { return handle; }

  // Checks if shader object handles an actual shader.
  // Should return false when called after the default constructor.
  bool exists() noexcept {
    // May already be enough when handle is not used from the outside.
    // return handle;
    // But this approach is more secure.
    return glIsShader(handle) == GL_TRUE;
  }

 private:
  void receive_handle() {
    handle = glCreateShader(shader_object_type);
    if (!handle)
      throw runtime_error(string("Failed to receive handle for ") +
                          type_name() + " shader object.");
  }

  bool compile_failed() noexcept {
    GLint compile_status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);
    return compile_status == GL_FALSE;
  }

  void write_compile_info_log(string& info_log) {
    GLint info_log_size;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &info_log_size);
    info_log.resize(info_log_size);
    if (info_log_size)
      glGetShaderInfoLog(handle, info_log_size, nullptr, info_log.data());
  }

  void throw_compile_error() {
    throw compile_error(string("Failed to compile ") + type_name() +
                        " shader object.");
  }

  void throw_compile_error(const string& info_log) {
    throw compile_error(string("Failed to compile ") + type_name() +
                        " shader object.\n" + info_log);
  }

  void compile(czstring source) {
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);
  }

  void compile(czstring source, string& info_log) {
    compile(source);
    write_compile_info_log(info_log);
    if (compile_failed()) throw_compile_error(info_log);
  }

  void compile(czstring source, warnings_as_errors_t) {
    compile(source);
    string info_log;
    write_compile_info_log(info_log);
    if (compile_failed() || !info_log.empty()) throw_compile_error(info_log);
  }

  void compile(czstring source, ignore_warnings_t) {
    compile(source);
    if (compile_failed()) {
      string info_log;
      write_compile_info_log(info_log);
      throw_compile_error(info_log);
    }
  }

  void compile(czstring source, auto&& warning_callback) {
    string info_log;
    compile(source, info_log);
    if (!info_log.empty())
      std::forward<decltype(warning_callback)>(warning_callback)(info_log);
  }

  GLuint handle{};
};

using vertex_shader = shader_object<GL_VERTEX_SHADER>;
using geometry_shader = shader_object<GL_GEOMETRY_SHADER>;
using fragment_shader = shader_object<GL_FRAGMENT_SHADER>;

class shader_program {
 public:
  struct link_error : runtime_error {
    using base = runtime_error;
    link_error(auto&& x) : base(std::forward<decltype(x)>(x)) {}
  };

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
    throw link_error("Failed to link shader program.");
  }

  void throw_link_error(const string& info_log) {
    throw link_error("Failed to link shader program.\n" + info_log);
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

}  // namespace viewer
