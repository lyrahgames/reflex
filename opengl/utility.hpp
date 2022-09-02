#pragma once
#include <numbers>
#include <stdexcept>
#include <string>
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

}  // namespace opengl
