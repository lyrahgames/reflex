#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
//
#include <glbinding/gl/gl.h>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>

namespace viewer {

using namespace std;
using clock = chrono::high_resolution_clock;
using time_type = decltype(clock::now());
using chrono::duration;

using namespace gl;

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

}  // namespace viewer
