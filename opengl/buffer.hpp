#pragma once
#include <opengl/utility.hpp>

namespace opengl {

template <auto buffer_type>
class buffer {
 public:
  buffer() { glGenBuffers(1, &handle); }
  ~buffer() { glDeleteBuffers(1, &handle); }

  // Copying is not allowed.
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;

  // Moving
  buffer(buffer&& x) : handle{x.handle} { x.handle = 0; }
  buffer& operator=(buffer&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

  void bind() const { glBindBuffer(buffer_type, handle); }

  // private:
  GLuint handle{};  // value zero is ignored
};

using vertex_buffer = buffer<GL_ARRAY_BUFFER>;
using element_buffer = buffer<GL_ELEMENT_ARRAY_BUFFER>;

}  // namespace opengl
