#pragma once
#include <opengl/utility.hpp>

namespace opengl {

template <auto texture_type>
class texture {
 public:
  texture() { glGenTextures(1, &handle); }
  ~texture() { glDeleteTextures(1, &handle); }

  // Copying is not allowed.
  texture(const texture&) = delete;
  texture& operator=(const texture&) = delete;

  // Moving
  texture(texture&& x) : handle{x.handle} { x.handle = 0; }
  texture& operator=(texture&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

  void bind() const { glBindTexture(texture_type, handle); }

  // private:
  GLuint handle{};  // value zero is ignored
};

using texture2 = texture<GL_TEXTURE_2D>;
using texture3 = texture<GL_TEXTURE_3D>;

using texture_handle = GLuint;

}  // namespace opengl
