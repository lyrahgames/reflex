#pragma once
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>
//
#include <lyrahgames/xstd/builtin_types.hpp>

namespace opengl {

using namespace std;
constexpr auto pi = std::numbers::pi_v<float>;

using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using namespace lyrahgames::xstd;

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

}  // namespace opengl
