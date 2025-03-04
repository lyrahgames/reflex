#pragma once
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
//
#include <concepts>
#include <ranges>
#include <type_traits>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>
//
#include <glm/gtx/norm.hpp>
//
#include <lyrahgames/xstd/named_tuple.hpp>
#include <lyrahgames/xstd/regular_tuple.hpp>
#include <lyrahgames/xstd/utility.hpp>

namespace opengl {

using namespace std;
constexpr auto pi = std::numbers::pi_v<float>;

using glm::distance2;

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::mat2;
using glm::mat3;
using glm::mat4;

using glm::mat2x3;
using glm::mat2x4;
using glm::mat3x2;
using glm::mat3x4;
using glm::mat4x2;
using glm::mat4x3;

using namespace lyrahgames::xstd;

namespace generic {

using namespace lyrahgames::xstd::generic;

template <typename T>
concept transferable = is_trivially_copyable_v<T>;
}  // namespace generic

// 'czstring' is used because the given file path is only read once.
inline auto string_from_file(czstring file_path) -> string {
  // We will read all characters as block.
  // Hence, open the file in binary mode.
  // Make sure to jump to its end for directly reading its size.
  ifstream file{file_path, ios::binary | ios::ate};
  if (!file)
    throw runtime_error("Failed to open the file '"s + file_path + "'.");
  // Read the file size.
  auto size = file.tellg();
  // Prepare the result string with a sufficiently large buffer.
  string result(size, '\0');
  // Go back to the start and read all characters at once.
  file.seekg(0);
  file.read(result.data(), size);
  return result;
};

// Need uniform interface to handle variable.
// Need to inherit from standards.
// So, the wrapper of a handle is useful.
template <typename T>
struct basic_handle {
  using type = T;

  constexpr basic_handle() noexcept = default;
  constexpr basic_handle(type value) noexcept : handle{value} {}

  constexpr operator type() noexcept { return handle; }

 protected:
  type handle{};
};

using object_handle = basic_handle<GLuint>;

// Why reference?
// Only using the handle itself will not allow you to use custom typed functions.
// The handle itself will never statically know about bindings.
// reference template will only store the handle without being able to delete it
// base class should be a handle class
// no default constructor allowed
// but default destructor is mandatory
// due to inheritance, every function from the base handle can be called
template <typename T>
struct reference : T::base {
  using base = typename T::base;
  reference(T& t) : base{t} {}
};

// Meta functions to transfer builtin types to their common OpenGL enum value.
namespace detail {
template <typename T>
struct common_enum_value {};
template <>
struct common_enum_value<GLbyte> {
  static constexpr GLenum value = GL_BYTE;
};
template <>
struct common_enum_value<GLubyte> {
  static constexpr GLenum value = GL_UNSIGNED_BYTE;
};
template <>
struct common_enum_value<GLshort> {
  static constexpr GLenum value = GL_SHORT;
};
template <>
struct common_enum_value<GLushort> {
  static constexpr GLenum value = GL_UNSIGNED_SHORT;
};
template <>
struct common_enum_value<GLint> {
  static constexpr GLenum value = GL_INT;
};
template <>
struct common_enum_value<GLuint> {
  static constexpr GLenum value = GL_UNSIGNED_INT;
};
template <>
struct common_enum_value<GLfloat> {
  static constexpr GLenum value = GL_FLOAT;
};
template <>
struct common_enum_value<GLdouble> {
  static constexpr GLenum value = GL_DOUBLE;
};
}  // namespace detail

/// Meta function to transfer builtin types to their common OpenGL enum value.
template <typename type>
constexpr auto common_enum_value = detail::common_enum_value<type>::value;

// Meta functions to transfer OpenGL enum value to its common type.
namespace detail {
template <GLenum value>
struct common_type {};
template <>
struct common_type<GL_BYTE> {
  using type = GLbyte;
};
template <>
struct common_type<GL_UNSIGNED_BYTE> {
  using type = GLubyte;
};
template <>
struct common_type<GL_SHORT> {
  using type = GLshort;
};
template <>
struct common_type<GL_UNSIGNED_SHORT> {
  using type = GLushort;
};
template <>
struct common_type<GL_INT> {
  using type = GLint;
};
template <>
struct common_type<GL_UNSIGNED_INT> {
  using type = GLuint;
};
template <>
struct common_type<GL_FLOAT> {
  using type = GLfloat;
};
template <>
struct common_type<GL_DOUBLE> {
  using type = GLdouble;
};
}  // namespace detail

/// Meta function to transfer OpenGL enum value to its common type.
template <GLenum value>
using common_type = typename detail::common_type<value>::type;

}  // namespace opengl
