#pragma once
#include <fstream>
//
#include <libviewer/model.hpp>

namespace viewer {

struct stl_binary_format {
  using header = array<uint8_t, 80>;
  using size_type = uint32_t;
  using attribute_byte_count_type = uint16_t;

  struct alignas(1) triangle {
    vec3 normal;
    vec3 vertex[3];
  };

  static_assert(offsetof(triangle, normal) == 0);
  static_assert(offsetof(triangle, vertex[0]) == 12);
  static_assert(offsetof(triangle, vertex[1]) == 24);
  static_assert(offsetof(triangle, vertex[2]) == 36);
  static_assert(sizeof(triangle) == 48);
  static_assert(alignof(triangle) == 4);

  stl_binary_format() = default;

  stl_binary_format(czstring file_path) {
    std::fstream file{file_path, std::ios::in | std::ios::binary};
    if (!file.is_open()) throw runtime_error("Failed to open given STL file.");

    // We will ignore the header.
    // It has no specific use to us.
    file.ignore(sizeof(header));

    // Read number of triangles.
    size_type size;
    file.read((char*)&size, sizeof(size));

    // Due to padding and alignment issues,
    // we cannot read everything at once.
    // Hence, we use a simple loop for every triangle.
    triangles.resize(size);
    for (auto& t : triangles) {
      file.read((char*)&t, sizeof(triangle));
      // Ignore the attribute byte count.
      // There should not be any information anyway.
      file.ignore(sizeof(attribute_byte_count_type));
    }
  }

  vector<triangle> triangles{};
};

void transform(const stl_binary_format& stl_data, model& mesh) {
  std::unordered_map<vec3, size_t, decltype([](const auto& v) -> size_t {
                       return (bit_cast<uint32_t>(v.x) << 11) ^
                              (bit_cast<uint32_t>(v.y) << 5) ^
                              bit_cast<uint32_t>(v.z);
                     })>
      position_index{};
  position_index.reserve(stl_data.triangles.size());

  for (size_t i = 0; i < stl_data.triangles.size(); ++i) {
    model::face f{};
    const auto& normal = stl_data.triangles[i].normal;
    const auto& v = stl_data.triangles[i].vertex;
    for (size_t j = 0; j < 3; ++j) {
      const auto k = (j + 1) % 3;
      const auto l = (j + 2) % 3;
      const auto p = v[k] - v[j];
      const auto q = v[l] - v[j];
      // const auto weight = length(cross(p, q)) / dot(p, p) / dot(q, q);
      const auto n = cross(p, q) / dot(p, p) / dot(q, q);

      const auto it = position_index.find(v[j]);
      if (it == end(position_index)) {
        const int index = mesh.vertices.size();
        f[j] = index;
        position_index.emplace(v[j], index);
        // mesh.vertices.push_back({v[j], normal});
        // mesh.vertices.push_back({v[j], weight * normal});
        mesh.vertices.push_back({v[j], n});
        continue;
      }

      const auto index = it->second;
      f[j] = index;
      // mesh.vertices[index].normal += normal;
      // mesh.vertices[index].normal += weight * normal;
      mesh.vertices[index].normal += n;
    }

    mesh.faces.push_back(f);
  }

  for (auto& v : mesh.vertices) {
    v.normal = normalize(v.normal);
  }
}

}  // namespace viewer
