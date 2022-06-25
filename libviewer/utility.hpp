#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>
//
#include <lyrahgames/xstd/builtin_types.hpp>

namespace viewer {

using namespace std;
using clock = chrono::high_resolution_clock;
using time_type = decltype(clock::now());
using chrono::duration;

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using namespace lyrahgames::xstd;

}  // namespace viewer
