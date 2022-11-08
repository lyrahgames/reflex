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
  struct edge_info {
    edge_info() {
      face_id[0] = size_t(-1);
      face_id[1] = size_t(-1);
    }

    void add_face(size_t fid, uint32 id) {
      if (face_id[0] == size_t(-1)) {
        face_id[0] = fid;
        location[0] = id;
        return;
      }

      face_id[1] = fid;
      location[1] = id;
    }

    bool inside() const noexcept { return face_id[1] != size_t(-1); }

    size_t face_id[2];
    uint32 location[2];
  };

  struct dual_info {
    size_t edge[2];
    size_t vid[2];
  };

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
    for (size_t i = 0; i < faces.size(); ++i) {
      const auto& f = faces[i];
      edges[pair{min(f[0], f[1]), max(f[0], f[1])}].add_face(i, 2);
      edges[pair{min(f[1], f[2]), max(f[1], f[2])}].add_face(i, 0);
      edges[pair{min(f[2], f[0]), max(f[2], f[0])}].add_face(i, 1);
    }

    // for (const auto& [e, info] : edges) {
    //   if (!info.inside()) continue;
    //   auto& d = dual_edges[pair{min(info.face_id[0], info.face_id[1]),
    //                             max(info.face_id[0], info.face_id[1])}];
    //   d.edge = {e.first, e.second};
    //   d.vid = {};
    // }
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
      const auto& n = vertices[i].normal;
      const auto& p = vertices[i].position;

      for (size_t j = neighbor_offset[i] + 1; j < neighbor_offset[i + 1]; ++j) {
        const auto vid1 = neighbors[j - 1];
        const auto v1 = vertices[vid1].position - p;

        bool has_neighbor = false;
        for (size_t k = j; k < neighbor_offset[i + 1]; ++k) {
          const auto vid2 = neighbors[k];
          const auto v2 = vertices[vid2].position - p;

          if (edges.contains(pair(min(vid1, vid2), max(vid1, vid2))) &&
              (dot(n, cross(v1, v2)) > 0.0f)) {
            swap(neighbors[k], neighbors[j]);
            has_neighbor = true;
            break;
          }
        }
        if (!has_neighbor)
          cout << i << " " << (j - neighbor_offset[i]) << " no neighbor."
               << endl;
      }

      for (size_t j = neighbor_offset[i] + 1; j < neighbor_offset[i + 1]; ++j) {
        if (!edges.contains(pair(min(neighbors[j - 1], neighbors[j]),
                                 max(neighbors[j - 1], neighbors[j])))) {
          // const auto v1 = vertices[neighbors[j - 1]].position - p;
          // const auto v2 = vertices[neighbors[j]].position - p;
          // if (dot(n, cross(v1, v2)) > 0.0f) continue;
          cout << "Failed to sort neighbors: " << i << ": "
               << (j - neighbor_offset[i]) << endl;
          break;
        }
      }
    }

    // for (size_t i = 0; i < vertices.size(); ++i) {
    //   assert(neighbor_count[i] == neighbor_offset[i + 1] - neighbor_offset[i]);
    // cout << i << ": " << neighbor_offset[i + 1] - neighbor_offset[i] << ": ";
    // for (size_t j = neighbor_offset[i]; j < neighbor_offset[i + 1]; ++j)
    //   cout << neighbors[j] << ", ";
    // cout << endl;
    // }

    face_neighbors.resize(faces.size());
    for (const auto& [e, info] : edges) {
      assert(info.face_id[0] < faces.size());

      face_neighbors[info.face_id[0]][info.location[0]] = info.face_id[1];
      if (!info.inside()) continue;
      face_neighbors[info.face_id[1]][info.location[1]] = info.face_id[0];

      assert(faces[info.face_id[0]][info.location[0]] != e.first);
      assert(faces[info.face_id[0]][info.location[0]] != e.second);

      assert(min(faces[info.face_id[0]][(info.location[0] + 1) % 3],
                 faces[info.face_id[0]][(info.location[0] + 2) % 3]) ==
             e.first);
      assert(max(faces[info.face_id[0]][(info.location[0] + 1) % 3],
                 faces[info.face_id[0]][(info.location[0] + 2) % 3]) ==
             e.second);

      assert(faces[info.face_id[1]][info.location[1]] != e.first);
      assert(faces[info.face_id[1]][info.location[1]] != e.second);

      assert(min(faces[info.face_id[1]][(info.location[1] + 1) % 3],
                 faces[info.face_id[1]][(info.location[1] + 2) % 3]) ==
             e.first);
      assert(max(faces[info.face_id[1]][(info.location[1] + 1) % 3],
                 faces[info.face_id[1]][(info.location[1] + 2) % 3]) ==
             e.second);
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

  auto compute_shortest_path_fast(size_t src, size_t dst) const
      -> vector<size_t> {
    vector<bool> visited(vertices.size(), false);

    vector<float> distances(vertices.size(), INFINITY);
    distances[src] = 0;

    vector<size_t> previous(vertices.size());
    previous[src] = src;

    vector<size_t> queue{src};
    const auto order = [&](size_t i, size_t j) {
      return distances[i] > distances[j];
    };

    do {
      ranges::make_heap(queue, order);
      ranges::pop_heap(queue, order);
      const auto current = queue.back();
      queue.pop_back();

      // cout << "current = " << current << endl;

      visited[current] = true;

      for (auto i = neighbor_offset[current];  //
           i < neighbor_offset[current + 1]; ++i) {
        const auto neighbor = neighbors[i];
        if (visited[neighbor]) continue;

        const auto d = distance(current, neighbor) + distances[current];
        if (d >= distances[neighbor]) continue;

        distances[neighbor] = d;
        previous[neighbor] = current;
        queue.push_back(neighbor);
      }
    } while (!queue.empty() && !visited[dst]);

    if (queue.empty()) return {};

    // cout << "dst visited" << endl;

    // Compute count and path.
    size_t count = 0;
    for (auto i = dst; i != src; i = previous[i]) ++count;
    vector<size_t> path(count);
    for (auto i = dst; i != src; i = previous[i]) path[--count] = i;
    return path;
  }

  auto compute_shortest_face_path_fast(size_t src, size_t dst) const
      -> vector<size_t> {
    const auto barycenter = [&](size_t fid) {
      const auto& f = faces[fid];
      return (vertices[f[0]].position + vertices[f[1]].position +
              vertices[f[2]].position) /
             3.0f;
    };
    const auto face_distance = [&](size_t i, size_t j) {
      return glm::distance(barycenter(i), barycenter(j));
    };

    vector<bool> visited(faces.size(), false);

    vector<float> distances(faces.size(), INFINITY);
    distances[src] = 0;

    vector<size_t> previous(faces.size());
    previous[src] = src;

    vector<size_t> queue{src};
    const auto order = [&](size_t i, size_t j) {
      return distances[i] > distances[j];
    };

    do {
      ranges::make_heap(queue, order);
      ranges::pop_heap(queue, order);
      const auto current = queue.back();
      queue.pop_back();

      // cout << "current = " << current << endl;

      visited[current] = true;

      const auto neighbor_faces = face_neighbors[current];
      for (size_t i = 0; i < 3; ++i) {
        const auto neighbor = neighbor_faces[i];
        if (visited[neighbor]) continue;

        const auto d = face_distance(current, neighbor) + distances[current];
        if (d >= distances[neighbor]) continue;

        distances[neighbor] = d;
        previous[neighbor] = current;
        queue.push_back(neighbor);
      }
    } while (!queue.empty() && !visited[dst]);

    if (queue.empty()) return {};

    // Compute count and path.
    size_t count = 0;
    for (auto i = dst; i != src; i = previous[i]) ++count;
    vector<size_t> path(count);
    for (auto i = dst; i != src; i = previous[i]) path[--count] = i;
    return path;
  }

  vector<vertex> vertices{};
  vector<face> faces{};
  int material_id = -1;

  static constexpr auto pair_hasher = [](const auto& x) {
    return (x.first << 7) ^ x.second;
  };
  unordered_map<pair<size_t, size_t>, edge_info, decltype(pair_hasher)> edges{};
  unordered_map<pair<size_t, size_t>, dual_info, decltype(pair_hasher)>
      dual_edges{};
  // map<pair<size_t, size_t>, int> edges{};
  vector<size_t> neighbor_offset{};
  vector<size_t> neighbors{};
  vector<array<size_t, 3>> face_neighbors{};
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
  struct vertex {
    vec3 position;
    vec3 normal;
    float arclength;
    float curvature;
  };

  points() noexcept { setup(); };

  void setup() {
    device_handle.bind();
    device_vertices.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, arclength));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, curvature));
  }

  void compute_length() {
    if (vertices.size() < 2) return;
    vertices[0].arclength = 0;

    for (size_t i = 1; i < vertices.size(); ++i)
      vertices[i].arclength =
          vertices[i - 1].arclength +
          distance(vertices[i - 1].position, vertices[i].position);

    for (size_t i = 1; i < vertices.size(); ++i)
      vertices[i].arclength /= vertices.back().arclength;
  }

  void compute_curvature() {
    if (vertices.size() < 3) return;

    float max_curvature = 0;

    for (size_t i = 1; i < vertices.size() - 1; ++i) {
      const auto& p = vertices[i].position;
      const auto& n = vertices[i].normal;

      const auto& a = vertices[i - 1].position;
      const auto& b = vertices[i + 1].position;

      const auto ap = p - a;
      const auto pb = b - p;

      const auto lap = 1.0f / length(ap);
      const auto lpb = 1.0f / length(pb);

      const auto u = lap * ap;
      const auto v = lpb * pb;

      const auto d = (u + v) / 2.0f;
      const auto d2 = (lpb * (v + u) + lap * (v - u)) / 2.0f;

      const auto k = glm::dot(d2, cross(n, d));
      max_curvature = max(abs(k), max_curvature);
      vertices[i].curvature = k;
    }

    for (size_t i = 1; i < vertices.size() - 1; ++i)
      vertices[i].curvature /= max_curvature;
  }

  void update() {
    compute_length();
    compute_curvature();
    device_vertices.bind();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex),
                 vertices.data(), GL_STATIC_DRAW);
  }

  void render() {
    device_handle.bind();
    // glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
    glDrawArrays(GL_POINTS, 0, vertices.size());
  }

  vector<vertex> vertices{};
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
  vector<float> curvature{};

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
      for (const auto& [e, info] : mesh.edges) {
        if (info.inside()) continue;
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
