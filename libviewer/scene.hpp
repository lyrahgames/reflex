#pragma once
#include <libviewer/utility.hpp>
//
#include <stb_image.h>

namespace viewer {

struct basic_material {
  string name{};
  string texture_path{};
};

struct material : basic_material {
  void bind(shader_program& shader) const noexcept {
    glActiveTexture(GL_TEXTURE0);
    shader.set("diffuse_texture", 0);
    glBindTexture(GL_TEXTURE_2D, device_texture);
  }

  texture_handle device_texture;
};

struct vertex {
  vec3 position{};
  vec3 normal{};
  vec2 uv{};
};

struct face : array<uint32_t, 3> {};
// using face = array<uint32_t, 3>;

struct basic_mesh {
  vector<vertex> vertices{};
  vector<face> faces{};
  int material_id = -1;
};

struct mesh : basic_mesh {
  static constexpr GLint position_attribute_location = 0;
  static constexpr GLint normal_attribute_location = 1;
  static constexpr GLint uv_attribute_location = 2;

  mesh() noexcept : basic_mesh() { setup(); }

  void setup() noexcept {
    device_handle.bind();
    device_vertices.bind();
    device_faces.bind();

    glEnableVertexAttribArray(position_attribute_location);
    glVertexAttribPointer(position_attribute_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, position));

    glEnableVertexAttribArray(normal_attribute_location);
    glVertexAttribPointer(normal_attribute_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, normal));

    glEnableVertexAttribArray(uv_attribute_location);
    glVertexAttribPointer(uv_attribute_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, uv));
  }

  void update() noexcept {
    device_vertices.bind();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex),
                 vertices.data(), GL_STATIC_DRAW);

    device_faces.bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(face),
                 faces.data(), GL_STATIC_DRAW);
  }

  void render() const noexcept {
    device_handle.bind();
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
  }

  vertex_array device_handle{};
  vertex_buffer device_vertices{};
  element_buffer device_faces{};
};

struct scene {
  scene() = default;

  void update() {
    for (auto& material : materials) {
      if (material.texture_path.empty()) {
        material.device_texture = 0;
        continue;
      }

      auto it = textures.find(material.texture_path);
      if (it != end(textures)) {
        material.device_texture = it->second.handle;
        continue;
      }

      int width = 0;
      int height = 0;
      int channels = 0;
      auto data = stbi_load(material.texture_path.c_str(), &width, &height,
                            &channels, 0);
      if (!data)
        throw runtime_error(string("Failed to load file '") +
                            material.texture_path + "' as image.");

      texture2 texture;
      texture.bind();

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,  //
                      GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      stbi_image_free(data);

      material.device_texture = texture.handle;
      textures.emplace(material.texture_path, move(texture));
    }

    for (auto& mesh : meshes) mesh.update();
  }

  void render(shader_program& shader) const noexcept {
    shader.bind();
    for (const auto& mesh : meshes) mesh.render();
  }

  vector<mesh> meshes{};
  vector<material> materials{};
  unordered_map<string, texture2> textures{};
};

}  // namespace viewer
