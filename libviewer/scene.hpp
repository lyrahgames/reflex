#pragma once
#include <libviewer/utility.hpp>
//
#include <stb_image.h>

namespace viewer {

struct basic_material {
  string name{};
  string texture_path{};
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct material : basic_material {
  static constexpr czstring glsl_type_code =
      "struct Material {"
      "  vec3 ambient;"
      "  vec3 diffuse;"
      "  vec3 specular;"
      "  float shininess;"
      "  sampler2D texture;"
      "};";

  void bind(shader_program& shader) const noexcept {
    glActiveTexture(GL_TEXTURE0);
    shader  //
        .set("material.texture", 0)
        .set("material.ambient", ambient)
        .set("material.diffuse", diffuse)
        .set("material.specular", specular)
        .set("material.shininess", shininess);
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
  scene() {
    // Generate default one-pixel texture which is white to indicate no texture.
    texture2 texture;
    texture.bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,  //
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const unsigned char data[] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 data);

    textures.emplace("", move(texture));
  }

  void update() {
    for (auto& material : materials) {
      // if (material.texture_path.empty()) {
      //   material.device_texture = 0;
      //   continue;
      // }

      auto it = textures.find(material.texture_path);
      if (it != end(textures)) {
        material.device_texture = it->second.handle;
        continue;
      }

      int width = 0;
      int height = 0;
      int channels = 0;
      // stbi_set_flip_vertically_on_load(1);
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
    shader.set("model", model_matrix);

    const auto normal_matrix = inverse(transpose(mat3(model_matrix)));
    shader.set("normal_matrix", normal_matrix);

    for (const auto& mesh : meshes) {
      materials[mesh.material_id].bind(shader);
      mesh.render();
    }
  }

  void animate(float dt) noexcept {
    model_matrix = rotate(model_matrix, 0.1f * dt, normalize(vec3(1, 1, 1)));
  }

  vector<mesh> meshes{};
  vector<material> materials{};
  unordered_map<string, texture2> textures{};
  mat4 model_matrix{1.0f};
  mat3 normal_matrix{1.0f};
};

}  // namespace viewer
