#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

struct ray {
  vec3 origin;
  vec3 direction;
};

struct triangle {
  vec3 vertex[3];
};

struct intersection {
  float u{};
  float v{};
  float t = INFINITY;
};

inline bool intersect(const ray& r,
                      const triangle& t,
                      intersection& uvt) noexcept {
  const auto edge1 = t.vertex[1] - t.vertex[0];
  const auto edge2 = t.vertex[2] - t.vertex[0];

  const auto p = cross(r.direction, edge2);
  const auto determinant = dot(edge1, p);
  const auto inverse_determinant = 1.0f / determinant;
  const auto s = r.origin - t.vertex[0];
  uvt.u = dot(s, p) * inverse_determinant;
  const auto q = cross(s, edge1);
  uvt.v = dot(r.direction, q) * inverse_determinant;
  uvt.t = dot(edge2, q) * inverse_determinant;

  return (0.0f != determinant) && (uvt.u >= 0.0f && uvt.u <= 1.0f) &&
         (uvt.v >= 0.0f && uvt.u + uvt.v <= 1.0f) && (uvt.t > 0.0f);
}

inline auto voronoi_snap(const triangle& t, const vec3& x) noexcept -> size_t {
  const auto a = distance2(x, t.vertex[0]);
  const auto b = distance2(x, t.vertex[1]);
  const auto c = distance2(x, t.vertex[2]);
  return (a <= b) ? ((a <= c) ? 0 : 2) : ((b <= c) ? 1 : 2);
}

}  // namespace viewer
