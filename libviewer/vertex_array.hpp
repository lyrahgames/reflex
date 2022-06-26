#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

class vertex_array {
 public:
  vertex_array() { glGenVertexArrays(1, &handle); }
  ~vertex_array() { glDeleteVertexArrays(1, &handle); }

  // Copying is not allowed.
  vertex_array(const vertex_array&) = delete;
  vertex_array& operator=(const vertex_array&) = delete;

  // Moving
  vertex_array(vertex_array&& x) : handle{x.handle} { x.handle = 0; }
  vertex_array& operator=(vertex_array&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

  void bind() const { glBindVertexArray(handle); }

  // private:
  GLuint handle{};  // value zero is ignored
};

}  // namespace viewer
