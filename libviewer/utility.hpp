#pragma once
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>
//
#include <opengl/opengl.hpp>

namespace viewer {

using namespace std;
using clock = chrono::high_resolution_clock;
using time_type = decltype(clock::now());
using chrono::duration;

using namespace opengl;

}  // namespace viewer
