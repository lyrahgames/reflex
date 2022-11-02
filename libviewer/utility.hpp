#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numbers>
#include <stdexcept>
#include <unordered_map>
#include <vector>
//
#include <string>
#include <string_view>
//
#include <atomic>
#include <future>
#include <mutex>
#include <thread>
//
#include <opengl/opengl.hpp>

namespace viewer {

using namespace std;
using clock = chrono::high_resolution_clock;
using time_type = decltype(clock::now());
using chrono::duration;

using namespace opengl;

inline auto operator<<(ostream& os, vec3 v) -> ostream& {
  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

}  // namespace viewer
