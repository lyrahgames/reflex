#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

struct vertex {
  vec3 position;
  vec3 normal;
};

struct face : array<uint32_t, 3> {};
// using face = array<uint32_t, 3>;

struct mesh {
  vector<vertex> vertices{};
  vector<face> faces{};
};

struct scene {
  vector<mesh> meshes{};
};

struct device_mesh {
  static constexpr GLint position_attribute_location = 0;
  static constexpr GLint normal_attribute_location = 1;

  device_mesh() noexcept { setup(); }
  device_mesh(const mesh& host) noexcept : device_mesh{} { load(host); }

  void setup() noexcept {
    handle.bind();
    vertices.bind();
    faces.bind();

    glEnableVertexAttribArray(position_attribute_location);
    glVertexAttribPointer(position_attribute_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, position));

    glEnableVertexAttribArray(normal_attribute_location);
    glVertexAttribPointer(normal_attribute_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, normal));
  }

  void load(const mesh& host) noexcept {
    vertices.bind();
    vertex_count = host.vertices.size();
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex),
                 host.vertices.data(), GL_STATIC_DRAW);

    faces.bind();
    face_count = host.faces.size();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_count * sizeof(face),
                 host.faces.data(), GL_STATIC_DRAW);
  }

  void render() const noexcept {
    handle.bind();
    glDrawElements(GL_TRIANGLES, 3 * face_count, GL_UNSIGNED_INT, 0);
  }

  vertex_array handle{};
  vertex_buffer vertices{};
  element_buffer faces{};
  size_t vertex_count = 0;
  size_t face_count = 0;
};

struct device_scene {
  device_scene() = default;
  device_scene(const scene& host) { load(host); }

  void load(const scene& host) {
    meshes.resize(host.meshes.size());
    for (size_t i = 0; i < host.meshes.size(); ++i)  //
      meshes[i].load(host.meshes[i]);
  }

  void render() const noexcept {
    for (const auto& mesh : meshes) mesh.render();
  }

  vector<device_mesh> meshes{};
};

}  // namespace viewer
