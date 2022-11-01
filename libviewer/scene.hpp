#pragma once
#include <libviewer/utility.hpp>
//
#include <stb_image.h>
//
#include <libviewer/intersection.hpp>

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
        .try_set("material.texture", 0)
        .try_set("material.ambient", ambient)
        .try_set("material.diffuse", diffuse)
        .try_set("material.specular", specular)
        .try_set("material.shininess", shininess);
    glBindTexture(GL_TEXTURE_2D, device_texture);
  }

  texture_handle device_texture;
};

struct vertex {
  vec3 position{};
  vec3 normal{};
  vec2 uv{};
};

using vertex_data =
    named_tuple<static_identifier_list<"position", "normal", "uv">,
                regular_tuple<vec3, vec3, vec2>>;

struct face : array<uint32_t, 3> {};
// using face = array<uint32_t, 3>;

struct basic_mesh {
  struct intersection : viewer::intersection {
    operator bool() const noexcept { return face_id != -1; }
    size_t face_id = -1;
  };

  auto intersect(const ray& r) -> intersection {
    intersection result{};
    for (size_t i = 0; i < faces.size(); ++i) {
      viewer::intersection uvt{};
      const auto intersected =
          viewer::intersect(r,
                            triangle{vertices[faces[i][0]].position,
                                     vertices[faces[i][1]].position,
                                     vertices[faces[i][2]].position},
                            uvt);

      if (!intersected) continue;
      if (uvt.t >= result.t) continue;

      result.face_id = i;
      result.u = uvt.u;
      result.v = uvt.v;
      result.t = uvt.t;
    }
    return result;
  }

  void compute_edges() {
    edges.clear();
    for (const auto& f : faces) {
      ++edges[pair{min(f[0], f[1]), max(f[0], f[1])}];
      ++edges[pair{min(f[1], f[2]), max(f[1], f[2])}];
      ++edges[pair{min(f[2], f[0]), max(f[2], f[0])}];
    }
  }

  void compute_neighbors() {
    neighbor_offset.resize(vertices.size() + 1);
    neighbor_offset[0] = 0;

    for (const auto& [e, _] : edges) {
      ++neighbor_offset[e.first + 1];
      ++neighbor_offset[e.second + 1];
    }

    for (size_t i = 2; i <= vertices.size(); ++i)
      neighbor_offset[i] += neighbor_offset[i - 1];

    vector<size_t> neighbor_count(vertices.size(), 0);
    neighbors.resize(neighbor_offset.back());

    for (const auto& [e, _] : edges) {
      neighbors[neighbor_offset[e.first] + neighbor_count[e.first]++] =
          e.second;
      neighbors[neighbor_offset[e.second] + neighbor_count[e.second]++] =
          e.first;
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
      assert(neighbor_count[i] == neighbor_offset[i + 1] - neighbor_offset[i]);
      // cout << i << ": " << neighbor_offset[i + 1] - neighbor_offset[i] << ": ";
      // for (size_t j = neighbor_offset[i]; j < neighbor_offset[i + 1]; ++j)
      //   cout << neighbors[j] << ", ";
      // cout << endl;
    }
  }

  auto distance(size_t x, size_t y) const noexcept -> float {
    return glm::distance(vertices[x].position, vertices[y].position);
  }

  auto compute_shortest_path(size_t src_vid, size_t dst_vid) const
      -> vector<size_t> {
    vector<bool> visited(vertices.size(), false);
    vector<float> distances(vertices.size(), INFINITY);
    vector<size_t> previous(vertices.size());
    vector<size_t> count(vertices.size());
    distances[src_vid] = 0;
    previous[src_vid] = src_vid;
    count[src_vid] = 0;

    size_t current = src_vid;
    float min_distance = INFINITY;

    do {
      for (size_t i = neighbor_offset[current];  //
           i < neighbor_offset[current + 1]; ++i) {
        const auto neighbor = neighbors[i];
        if (visited[neighbor]) continue;
        const auto d = distance(current, neighbor) + distances[current];
        if (d >= distances[neighbor]) continue;
        distances[neighbor] = d;
        previous[neighbor] = current;
        count[neighbor] = count[current] + 1;
      }
      visited[current] = true;

      min_distance = INFINITY;
      for (size_t i = 0; i < vertices.size(); ++i) {
        if (visited[i]) continue;
        if (distances[i] >= min_distance) continue;
        min_distance = distances[i];
        current = i;
      }

    } while (!visited[dst_vid] && (min_distance < INFINITY));

    if (min_distance == INFINITY) return {};

    // Backtrack the path.
    vector<size_t> path(count[dst_vid]);
    current = dst_vid;
    size_t index = path.size() - 1;
    do {
      path[index--] = current;
      current = previous[current];
    } while (current != src_vid);
    return path;
  }

  vector<vertex> vertices{};
  vector<face> faces{};
  int material_id = -1;

  // unordered_map<pair<size_t, size_t>, int, decltype([](const auto& x) {
  //                 return (x.first << 7) ^ x.second;
  //               })>
  //     edges{};
  map<pair<size_t, size_t>, int> edges{};
  vector<size_t> neighbor_offset{};
  vector<size_t> neighbors{};
};

struct mesh : basic_mesh {
  static constexpr GLint position_attribute_location = 0;
  static constexpr GLint normal_attribute_location = 1;
  static constexpr GLint uv_attribute_location = 2;

  mesh() noexcept : basic_mesh() { setup(); }

  void setup() noexcept {
    // device_handle.bind();
    device_vertices.bind();
    device_handle.template setup_aos<vertex_data>();
    device_faces.bind();

    // glEnableVertexAttribArray(position_attribute_location);
    // glVertexAttribPointer(position_attribute_location, 3, GL_FLOAT, GL_FALSE,
    //                       sizeof(vertex), (void*)offsetof(vertex, position));

    // glEnableVertexAttribArray(normal_attribute_location);
    // glVertexAttribPointer(normal_attribute_location, 3, GL_FLOAT, GL_FALSE,
    //                       sizeof(vertex), (void*)offsetof(vertex, normal));

    // glEnableVertexAttribArray(uv_attribute_location);
    // glVertexAttribPointer(uv_attribute_location, 2, GL_FLOAT, GL_FALSE,
    //                       sizeof(vertex), (void*)offsetof(vertex, uv));
  }

  void update() noexcept {
    // device_vertices.bind();
    // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex),
    //              vertices.data(), GL_STATIC_DRAW);

    // device_faces.bind();
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(face),
    //              faces.data(), GL_STATIC_DRAW);

    // device_vertices.initialize(vertices.data(), vertices.size());
    // device_faces.initialize(faces.data(), faces.size());

    device_vertices.allocate_and_initialize(vertices);
    device_faces.allocate_and_initialize(faces);

    // device_vertices = vertices;
    // device_faces = faces;
  }

  void render() const noexcept {
    device_handle.bind();
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
  }

  vertex_array device_handle{};
  vertex_buffer device_vertices{};
  element_buffer device_faces{};
};

struct points {
  points() noexcept { setup(); };

  void setup() {
    device_handle.bind();
    device_vertices.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
  }

  void update() {
    device_vertices.bind();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
                 vertices.data(), GL_STATIC_DRAW);
  }

  void render() {
    device_handle.bind();
    // glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
    glDrawArrays(GL_POINTS, 0, vertices.size());
  }

  vector<vec3> vertices{};
  vertex_array device_handle{};
  vertex_buffer device_vertices{};
};

struct lines {
  lines() noexcept { setup(); }

  void setup() const noexcept {
    device_handle.bind();
    device_vertices.bind();
    // device_lines.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
  }

  void update() const noexcept {
    device_vertices.allocate_and_initialize(vertices);
    // device_faces.allocate_and_initialize(lines);
  }

  void render() const noexcept {
    device_handle.bind();
    // glDrawElements(GL_LINES, 2 * lines.size(), GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_LINES, 0, vertices.size());
  }

  vector<vec3> vertices{};
  // vector<array<uint32_t, 2>> lines{};

  vertex_array device_handle;
  vertex_buffer device_vertices;
  element_buffer device_lines;
};

// struct marked_triangle {
//   marked_triangle(const mesh& m, size_t face_id) noexcept : mesh_ref{m} {
//     device_handle.bind();
//     device_vertices.bind();

//     face f{m.vertices[m.faces[face_id][0]],  //
//            m.vertices[m.faces[face_id][1]],  //
//            m.vertices[m.faces[face_id][2]]};

//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), m.faces[face_id].data(),
//                  GL_STATIC_DRAW);
//   }

//   void render() const noexcept {
//     device_handle.bind();
//     glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
//   }

//   vertex_array device_handle{};
//   vertex_buffer device_vertices{};
// };

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

    boundaries.resize(meshes.size());
    for (size_t i = 0; i < meshes.size(); ++i) {
      auto& mesh = meshes[i];
      mesh.update();
      mesh.compute_edges();
      mesh.compute_neighbors();

      auto& boundary = boundaries[i];
      for (const auto& [e, triangles] : mesh.edges) {
        if (triangles != 1) continue;
        const auto [p, q] = e;
        boundary.vertices.push_back(mesh.vertices[p].position);
        boundary.vertices.push_back(mesh.vertices[q].position);
      }
      boundary.update();
    }
  }

  void set_uniforms(shader_program& shader) const noexcept {
    shader.bind();
    shader.try_set("model", model_matrix);

    const auto normal_matrix = inverse(transpose(mat3(model_matrix)));
    shader.try_set("normal_matrix", normal_matrix);
  }

  void render(shader_program& shader, const mesh& m) const noexcept {
    set_uniforms(shader);
    materials[m.material_id].bind(shader);
    m.render();
  }

  void render(shader_program& shader) const noexcept {
    set_uniforms(shader);
    for (const auto& mesh : meshes) {
      materials[mesh.material_id].bind(shader);
      mesh.render();
    }
  }

  void render_boundaries() const noexcept {
    for (const auto& boundary : boundaries) boundary.render();
  }

  void animate(float dt) noexcept {
    // model_matrix = rotate(model_matrix, 0.1f * dt, normalize(vec3(1, 1, 1)));
  }

  struct intersection : mesh::intersection {
    operator bool() const noexcept { return mesh_id != -1; }
    size_t mesh_id = -1;
  };

  auto intersect(const ray& r) -> intersection {
    intersection result{};
    for (size_t i = 0; i < meshes.size(); ++i) {
      const auto p = meshes[i].intersect(r);
      if (!p) continue;
      if (p.t >= result.t) continue;
      result.mesh_id = i;
      result.face_id = p.face_id;
      result.u = p.u;
      result.v = p.v;
      result.t = p.t;
    }
    return result;
  }

  vector<mesh> meshes{};
  vector<lines> boundaries{};
  vector<material> materials{};
  unordered_map<string, texture2> textures{};
  mat4 model_matrix{1.0f};
  mat3 normal_matrix{1.0f};
};

}  // namespace viewer
