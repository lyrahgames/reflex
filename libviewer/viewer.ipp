#include <libviewer/viewer.hpp>
//
#include <libviewer/loader.hpp>
//
#include <stb_image.h>
//
#include <libviewer/contours_shader.hpp>
#include <libviewer/default_shader.hpp>
#include <libviewer/line_shader.hpp>
#include <libviewer/point_shader.hpp>
#include <libviewer/wireframe_shader.hpp>

namespace viewer {

viewer::viewer(int w, int h) : screen_width(w), screen_height(h) {
  // To initialize the viewport and matrices,
  // window has to be resized at least once.
  resize();

  // Setup for OpenGL
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_POINT_SMOOTH);
  // glEnable(GL_POINT_SPRITE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glPointSize(10.0f);
  glLineWidth(4.0f);

  shader = default_shader();
  selection_shader = wireframe_shader();
  point_selection_shader = point_shader();
  curve_shader = line_shader();

  // device_camera.bind();
  // glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(mat4), nullptr, GL_STATIC_DRAW);
  // glBindBufferBase(GL_UNIFORM_BUFFER, 0, device_camera);

  device_uniforms.set_binding(0).allocate(2 * sizeof(mat4));

  calls["exit"] = s.create([this]() { stop(); });
  calls["load_shader"] =
      s.create([this](string path) { load_shader(path.c_str()); });
  calls["load_model"] =
      s.create([this](string path) { load_model(path.c_str()); });

  calls["help"] = s.create([this] {
    for (const auto& [name, _] : calls) cout << name << endl;
  });
}

viewer::~viewer() {}

void viewer::resize() {
  glViewport(0, 0, screen_width, screen_height);
  cam.set_screen_resolution(screen_width, screen_height);
  view_should_update = true;
}

void viewer::resize(int width, int height) {
  screen_width = width;
  screen_height = height;
  resize();
}

void viewer::interpret_command(const string& line) {
  stringstream input{line};
  string command;
  input >> command;

  auto it = calls.find(command);
  if (it == calls.end()) cout << "Unknown function." << endl;
  // return "Unknown function";
  else {
    // stringstream s;
    auto [name, call] = *it;
    call(input, cout);
    // return s.str();
  }
}

void viewer::update() {
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }

  if (auto connection = server.accept()) {
    string line{};
    while ((line = connection.read()).empty()) {
    }
    interpret_command(line);
    // connection.write(result);
  }

  const auto new_time = clock::now();
  const auto dt = duration<float>(new_time - time).count();
  time = new_time;

  scene.animate(dt);
}

void viewer::render() {
  // Clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);
  scene.render(shader);

  curve_shader.bind();
  scene.render_boundaries();

  glDepthFunc(GL_ALWAYS);
  scene.render(selection_shader, selection);

  point_selection_shader.bind();
  point_selection.render();

  curve_shader.bind();
  point_selection.device_handle.bind();
  glDrawArrays(GL_LINE_STRIP, 0, point_selection.vertices.size());
}

void viewer::update_view() {
  // Computer camera position by using spherical coordinates.
  // This transformation is a variation of the standard
  // called horizontal coordinates often used in astronomy.
  auto p = cos(altitude) * sin(azimuth) * right -  //
           cos(altitude) * cos(azimuth) * front +  //
           sin(altitude) * up;
  p *= radius;
  p += origin;
  cam.move(p).look_at(origin, up);

  cam.set_near_and_far(std::max(1e-3f * radius, radius - bounding_radius),
                       radius + bounding_radius);

  // cout << "camera ar = " << cam.aspect_ratio() << '\n'
  //      << "camera fov = " << (pi / 180.0f * cam.vfov()) << '\n'
  //      << "scale = " << 1.0f / std::tan(cam.vfov() / 2) << '\n'
  //      << "camera pixel = " << cam.pixel_size() << '\n'
  //      << "camera near = " << cam.near() << '\n'
  //      << "camera far = " << cam.far() << '\n'
  //      << "camera position = " << cam.position() << '\n'
  //      << "camera direction = " << cam.direction() << '\n'
  //      << "camera right = " << cam.right() << '\n'
  //      << "camera up = " << cam.up() << '\n'
  //      << "inv translate = "
  //      << -vec3{dot(cam.right(), cam.position()), dot(cam.up(), cam.position()),
  //               dot(cam.front(), cam.position())}
  //      << '\n'
  //      << '\n';

  // const auto view_matrix = glm::lookAt(p, origin, up);
  // cout << "view = \n";
  // for (size_t i = 0; i < 4; ++i) {
  //   for (size_t j = 0; j < 4; ++j) cout << setw(15) << cam.view_matrix()[j][i];
  //   cout << '\n';
  // }

  // cout << "\nprojection = \n";
  // for (size_t i = 0; i < 4; ++i) {
  //   for (size_t j = 0; j < 4; ++j)
  //     cout << setw(15) << cam.projection_matrix()[j][i];
  //   cout << '\n';
  // }
  // cout << endl;

  // device_camera.bind();
  // glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4),
  //                 value_ptr(cam.projection_matrix()));
  // glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4),
  //                 value_ptr(cam.view_matrix()));
  // glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), sizeof(mat4),
  //                 value_ptr(cam.projection_matrix()));
  // glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(mat4), sizeof(mat4),
  //                 value_ptr(cam.view_matrix()));

  device_uniforms  //
      .write(cam.projection_matrix())
      .write(cam.view_matrix(), sizeof(mat4));

  // shader.bind();
  // shader.set("projection", cam.projection_matrix())
  //     .set("view", cam.view_matrix());

  shader.bind();
  shader  //
      .set("camera.projection", cam.projection_matrix())
      .set("camera.view", cam.view_matrix())
      .set("camera.viewport", cam.viewport_matrix());

  selection_shader.bind();
  selection_shader  //
      .set("camera.projection", cam.projection_matrix())
      .set("camera.view", cam.view_matrix())
      .set("camera.viewport", cam.viewport_matrix());

  point_selection_shader.bind();
  point_selection_shader  //
      .set("camera.projection", cam.projection_matrix())
      .set("camera.view", cam.view_matrix())
      .set("camera.viewport", cam.viewport_matrix());

  curve_shader.bind();
  curve_shader  //
      .set("camera.projection", cam.projection_matrix())
      .set("camera.view", cam.view_matrix())
      .set("camera.viewport", cam.viewport_matrix());
}

void viewer::turn(const vec2& angle) {
  altitude += angle.y;
  azimuth += angle.x;
  constexpr float bound = pi / 2 - 1e-5f;
  altitude = std::clamp(altitude, -bound, bound);
  view_should_update = true;
}

void viewer::shift(const vec2& pixels) {
  const auto shift = -pixels.x * cam.right() + pixels.y * cam.up();
  const auto scale = cam.pixel_size() * radius;
  origin += scale * shift;
  view_should_update = true;
}

void viewer::zoom(float scale) {
  radius *= exp(-scale);
  view_should_update = true;
}

void viewer::set_z_as_up() {
  right = {1, 0, 0};
  front = {0, -1, 0};
  up = {0, 0, 1};
  view_should_update = true;
}

void viewer::set_y_as_up() {
  right = {1, 0, 0};
  front = {0, 0, 1};
  up = {0, 1, 0};
  view_should_update = true;
}

void viewer::fit_view() {
  // AABB computation
  aabb_min = {INFINITY, INFINITY, INFINITY};
  aabb_max = {-INFINITY, -INFINITY, -INFINITY};
  for (auto& mesh : scene.meshes) {
    for (const auto& vertex : mesh.vertices) {
      aabb_min = min(aabb_min, vertex.position);
      aabb_max = max(aabb_max, vertex.position);
    }
  }

  origin = 0.5f * (aabb_max + aabb_min);
  bounding_radius = 0.5f * length(aabb_max - aabb_min);
  radius = 0.5f * length(aabb_max - aabb_min) / tan(0.5f * cam.vfov());
  cam.set_near_and_far(1e-4f * radius, 2 * radius);
  view_should_update = true;
}

void viewer::load_model(czstring file_path) {
  loader l;
  l.load(file_path, scene);

  // for (size_t id = 0; auto& mesh : scene.meshes) {
  //   cout << "Mesh " << id << ":\n"
  //        << "  material_id = " << mesh.material_id << '\n'
  //        << endl;
  //   ++id;
  // }

  // for (size_t id = 0; auto& material : scene.materials) {
  //   cout << "Material " << id << ":\n"
  //        << "  name = " << material.name << '\n'
  //        << "  texture = " << material.texture_path << '\n'
  //        << "  ambient = " << material.ambient << '\n'
  //        << "  diffuse = " << material.diffuse << '\n'
  //        << "  specular = " << material.specular << '\n'
  //        << "  shininess = " << material.shininess << '\n'
  //        << endl;
  //   ++id;
  // }

  // for (size_t id = 0; auto& texture : scene.textures) {
  //   cout << "Texture " << id << ":\n"
  //        << "  path = " << texture.path << '\n'
  //        << endl;
  //   ++id;
  // }

  fit_view();
  scene.update();
  view_should_update = true;

  // cout << "edges = " << scene.meshes[0].edges.size() << endl;

  cout << "vertices = " << scene.meshes[0].vertices.size() << endl
       << "faces = " << scene.meshes[0].faces.size() << endl;
}

void viewer::load_shader(czstring path) {
  // if (filesystem::is_directory(filesystem::path(path))) {
  //   constexpr czstring vertex_shader_name = "vs.glsl";
  //   constexpr czstring geometry_shader_name = "gs.glsl";
  //   constexpr czstring fragment_shader_name = "fs.glsl";

  //   return;
  // }

  // const auto file_content = [](czstring path) {
  //   ifstream file{path, ios::binary | ios::ate};
  //   if (!file) throw runtime_error("Failed to open file.");
  //   auto size = file.tellg();
  //   string content(size, '\0');
  //   file.seekg(0);
  //   file.read(content.data(), size);
  //   return content;
  // };

  // auto vs_path = filesystem::path(path);
  // vs_path += ".vert";
  // if (!filesystem::is_regular_file(vs_path))
  //   throw runtime_error("Vertex shader file does not exist.");
  // // const auto vs_code = file_content(vs_path.c_str());
  // const auto vs = vertex_shader_from_file(vs_path.c_str());

  // auto fs_path = filesystem::path(path);
  // fs_path += ".frag";
  // if (!filesystem::is_regular_file(fs_path))
  //   throw runtime_error("Fragment shader file does not exist.");
  // // const auto fs_code = file_content(fs_path.c_str());
  // const auto fs = fragment_shader_from_file(fs_path.c_str());

  // auto gs_path = filesystem::path(path);
  // gs_path += ".geom";
  // if (filesystem::is_regular_file(gs_path)) {
  //   // const auto gs_code = file_content(gs_path.c_str());
  //   const auto gs = geometry_shader_from_file(gs_path.c_str());
  //   shader = shader_program{vs, gs, fs};
  //   return;
  // }

  // shader = shader_program{vs, fs};

  shader = shader_from_file(path);
  view_should_update = true;

  const auto index = glGetUniformLocation(shader, "projection");
  cout << "index = " << index << endl;
  if (index == GL_INVALID_INDEX) cout << "index is invalid" << endl;
}

void viewer::select_face(float x, float y) {
  const auto direction = normalize(
      cam.direction() +
      cam.pixel_size() * ((x - 0.5f * cam.screen_width()) * cam.right() +
                          (0.5f * cam.screen_height() - y) * cam.up()));
  ray r{cam.position(), direction};
  const auto p = scene.intersect(r);

  if (p) {
    const auto& m = scene.meshes[p.mesh_id];
    const auto& f = m.faces[p.face_id];
    selection.vertices = {m.vertices[f[0]], m.vertices[f[1]], m.vertices[f[2]]};
    selection.faces = {{0, 1, 2}};
    selection.material_id = m.material_id;
    selection.update();
  }

  // cout << "x = " << x << '\n'
  //      << "y = " << y << '\n'
  //      << "mesh = " << p.mesh_id << '\n'
  //      << "face = " << p.face_id << '\n'
  //      << "u = " << p.u << '\n'
  //      << "v = " << p.v << '\n'
  //      << "t = " << p.t << '\n'
  //      << endl;
}

void viewer::select_vertex(float x, float y) {
  const auto direction = normalize(
      cam.direction() +
      cam.pixel_size() * ((x - 0.5f * cam.screen_width()) * cam.right() +
                          (0.5f * cam.screen_height() - y) * cam.up()));
  ray r{cam.position(), direction};
  const auto p = scene.intersect(r);

  // cout << "x = " << x << '\n'
  //      << "y = " << y << '\n'
  //      << "mesh = " << p.mesh_id << '\n'
  //      << "face = " << p.face_id << '\n'
  //      << "u = " << p.u << '\n'
  //      << "v = " << p.v << '\n'
  //      << "t = " << p.t << '\n'
  //      << endl;

  if (!p) return;

  const auto& m = scene.meshes[p.mesh_id];
  const auto& f = m.faces[p.face_id];
  const auto& v = m.vertices;

  // const size_t index = (p.u > p.v) ? ((p.u > (1 - p.u - p.v)) ? (1) : (0))
  //                                  : ((p.v > (1 - p.u - p.v)) ? (2) : (0));

  const auto position = r.origin + p.t * r.direction;

  curve_points.push_back({p.mesh_id, p.face_id, p.u, p.v, position});

  point_selection.vertices.push_back({position});
  point_selection.update();

  // cout << "curve point count = " << curve_points.size() << endl;
}

void viewer::check_curve_consistency() {
  const auto& mesh = scene.meshes[curve.mesh_id];
  const auto& vertices = curve.vertices;

  if (vertices.size() < 2) {
    cout << "Curve Consistency Check Failed: Curve contains too few points."
         << endl;
    return;
  }

  for (auto v : vertices) {
    if (v < mesh.vertices.size()) continue;
    cout << "Curve Consistency Check Failed: Curve contains invalid vertex IDs."
         << endl;
    return;
  }

  for (size_t i = 1; i < vertices.size(); ++i) {
    auto a = vertices[i];
    auto b = vertices[i - 1];
    if (mesh.edges.contains(pair(min(a, b), max(a, b)))) continue;
    cout << "Curve Consistency Check Failed: Curve contains adjacent point "
            "that are no neighbors."
         << endl;
    return;
  }

  for (size_t i = 2; i < vertices.size(); ++i) {
    auto a = vertices[i];
    auto b = vertices[i - 2];
    if ((a != b) && (!mesh.edges.contains(pair(min(a, b), max(a, b)))))
      continue;
    cout << "Curve Consistency Check Failed: Curve contains three adjacent "
            "point that are on the same triangle."
         << endl;
    return;
  }

  cout << "Curve Consistency Check Succeeded" << endl;
}

void viewer::preprocess_curve() {
  if (curve_points.empty()) return;

  curve.vertices.clear();

  const auto& p = curve_points[0];
  const auto& m = scene.meshes[p.mesh_id];
  const auto& f = m.faces[p.face_id];
  const auto& v = m.vertices;
  const auto index = voronoi_snap(
      {v[f[0]].position, v[f[1]].position, v[f[2]].position}, p.position);

  curve.mesh_id = p.mesh_id;
  curve.vertices.push_back(f[index]);

  size_t face_id = p.face_id;
  bool max_insert = false;

  for (size_t i = 1; i < curve_points.size(); ++i) {
    const auto& p = curve_points[i];

    if (p.mesh_id != curve.mesh_id) break;

    const auto& m = scene.meshes[p.mesh_id];
    const auto& f = m.faces[p.face_id];
    const auto& v = m.vertices;
    const auto id = voronoi_snap(
        {v[f[0]].position, v[f[1]].position, v[f[2]].position}, p.position);
    size_t vid = f[id];

    const auto a = curve.vertices.back();
    if (vid != a) {
      const auto path = m.compute_shortest_path_fast(a, vid);
      // If there is no path then no connection exists.
      if (path.empty()) break;
      for (auto x : path) curve.vertices.push_back(x);
    }
  }

  // Remove artifacts.
  // has to be done after path generation because start or end connection
  // to dijkstra path may not fulfill consistency requirement.
  {
    size_t curve_size = 2;  // First two points are always ok.
    for (size_t i = 2; i < curve.vertices.size(); ++i) {
      const auto vid = curve.vertices[i];
      // There always has to be one point in the curve.
      const auto a = curve.vertices[curve_size - 1];
      if (vid == a) continue;
      if (curve_size < 2) {
        curve.vertices[curve_size++] = vid;
        continue;
      }
      // By construction, a and b are not equal.
      const auto b = curve.vertices[curve_size - 2];
      if (vid == b) {
        // Remove last point. This could make the curve consist only of one point.
        --curve_size;
        continue;
      }
      // At this point, a must be a neighbor of vid and b by construction.
      if (m.edges.contains(pair{min(b, vid), max(b, vid)})) {
        // All points lie on triangle. Remove middle point.
        --curve_size;
        --i;
        continue;
      }
      curve.vertices[curve_size++] = vid;
    }
    curve.vertices.resize(curve_size);
  }

  // cout << "curve size = " << curve.vertices.size() << endl;

  check_curve_consistency();

  point_selection.vertices.clear();
  for (auto vid : curve.vertices) {
    const auto& m = scene.meshes[curve.mesh_id];
    const auto& v = m.vertices;
    point_selection.vertices.push_back({v[vid].position, v[vid].normal, 0, 0});
  }
  point_selection.update();
}

void viewer::preprocess_face_curve() {
  if (curve_points.empty()) return;

  face_curve.faces.clear();
  const auto& p = curve_points[0];
  face_curve.mesh_id = p.mesh_id;

  face_curve.faces.push_back(p.face_id);

  const auto& m = scene.meshes[p.mesh_id];

  for (size_t i = 1; i < curve_points.size(); ++i) {
    const auto& p = curve_points[i];
    if (p.mesh_id != face_curve.mesh_id) break;
    const auto fid = p.face_id;

    const auto a = face_curve.faces.back();
    if (fid == a) continue;
    // face_curve.faces.push_back(fid);

    const auto path = m.compute_shortest_face_path_fast(a, fid);
    // If there is no path then no connection exists.
    if (path.empty()) break;
    for (auto x : path) face_curve.faces.push_back(x);
  }

  // Remove artifacts
  {
    size_t curve_size = 2;
    for (size_t i = 2; i < face_curve.faces.size(); ++i) {
      const auto fid = face_curve.faces[i];
      // There always has to be one point in the face_curve.
      const auto a = face_curve.faces[curve_size - 1];
      if (fid == a) continue;
      if (curve_size < 2) {
        face_curve.faces[curve_size++] = fid;
        continue;
      }
      // By construction, a and b are not equal.
      const auto b = face_curve.faces[curve_size - 2];
      if (fid == b) {
        // Remove last point.
        // This could make the face_curve consist only of one point.
        --curve_size;
        continue;
      }
      // At this point, a must be a neighbor of fid and b by construction.
      face_curve.faces[curve_size++] = fid;
    }
    face_curve.faces.resize(curve_size);
  }

  if (face_curve.faces.size() < 2) return;

  // Generate points
  point_selection.vertices.clear();
  smooth_curve.mesh_id = face_curve.mesh_id;
  smooth_curve.vertices.clear();
  {
    const auto a = face_curve.faces[0];
    const auto b = face_curve.faces[1];
    size_t i = 0;
    for (; i < 3; ++i)
      if (m.face_neighbors[a][i] == b) break;
    const auto vid = m.faces[a][i];
    const auto& vertex = m.vertices[vid];
    point_selection.vertices.push_back({vertex.position, vertex.normal});
    smooth_curve.vertices.push_back({{vid, vid}, vertex.position, 0.0f});
  }
  for (size_t i = 1; i < face_curve.faces.size(); ++i) {
    const auto a = face_curve.faces[i - 1];
    const auto b = face_curve.faces[i];
    size_t k = 0;
    for (; k < 3; ++k)
      if (m.face_neighbors[b][k] == a) break;
    // size_t l = 0;
    // for (; l < 3; ++l)
    //   if (m.face_neighbors[a][l] == b) break;
    // if (pair(min(m.faces[b][(k + 1) % 3], m.faces[b][(k + 2) % 3]),
    //          max(m.faces[b][(k + 1) % 3], m.faces[b][(k + 2) % 3])) !=
    //     pair(min(m.faces[a][(l + 1) % 3], m.faces[a][(l + 2) % 3]),
    //          max(m.faces[a][(l + 1) % 3], m.faces[a][(l + 2) % 3])))
    //   cout << "fucking error!!" << endl;
    const auto vid1 = m.faces[b][(k + 1) % 3];
    const auto vid2 = m.faces[b][(k + 2) % 3];
    const auto position =
        (m.vertices[vid1].position + m.vertices[vid2].position) / 2.0f;
    const auto normal =
        normalize((m.vertices[vid1].normal + m.vertices[vid2].normal) / 2.0f);
    point_selection.vertices.push_back({position, normal});
    smooth_curve.vertices.push_back({{vid1, vid2}, position, 0.5f});
  }
  {
    const auto a = face_curve.faces[face_curve.faces.size() - 2];
    const auto b = face_curve.faces[face_curve.faces.size() - 1];
    size_t i = 0;
    for (; i < 3; ++i)
      if (m.face_neighbors[b][i] == a) break;
    const auto vid = m.faces[a][i];
    const auto& vertex = m.vertices[vid];
    point_selection.vertices.push_back({vertex.position, vertex.normal});
    smooth_curve.vertices.push_back({{vid, vid}, vertex.position, 0.0f});
  }

  // for (auto fid : face_curve.faces) {
  //   const auto& f = mesh.faces[fid];
  //   const auto& v = mesh.vertices;

  //   const auto position =
  //       (v[f[0]].position + v[f[1]].position + v[f[2]].position) / 3.0f;

  //   point_selection.vertices.push_back({position});
  // }
  point_selection.update();
}

void viewer::smooth_initial_curve() {
  if (smooth_curve.vertices.size() <= 2) return;

  const auto& mesh = scene.meshes[smooth_curve.mesh_id];

  auto vertices = smooth_curve.vertices;
  for (size_t i = 1; i < vertices.size() - 1; ++i) {
    const auto& x = vertices[i];
    const auto vid1 = x.edge[0];
    const auto vid2 = x.edge[1];
    if (vid1 == vid2) continue;
    const auto& prev = vertices[i - 1].position;
    const auto& next = vertices[i + 1].position;
    const auto& v1 = mesh.vertices[vid1].position;
    const auto& v2 = mesh.vertices[vid2].position;

    const auto u = v2 - v1;
    const auto u2 = dot(u, u);
    const auto t1 = dot(u, prev - v1) / u2;
    const auto t2 = dot(u, next - v1) / u2;

    const auto p1 = t1 * u + v1;
    const auto p2 = t2 * u + v1;

    const auto d1 = length(prev - p1);
    const auto d2 = length(next - p2);

    if (d1 / sqrt(u2) < 1e-3f) {
      const auto k = (t1 < 0.5f) ? 0 : 1;
      smooth_curve.vertices[i].edge[0] = x.edge[k];
      smooth_curve.vertices[i].edge[1] = x.edge[k];
      smooth_curve.vertices[i].position = mesh.vertices[x.edge[k]].position;
      smooth_curve.vertices[i].t = 0.0f;
      continue;
    }

    if (d2 / sqrt(u2) < 1e-3f) {
      const auto k = (t2 < 0.5f) ? 0 : 1;
      smooth_curve.vertices[i].edge[0] = x.edge[k];
      smooth_curve.vertices[i].edge[1] = x.edge[k];
      smooth_curve.vertices[i].position = mesh.vertices[x.edge[k]].position;
      smooth_curve.vertices[i].t = 0.0f;
      continue;
    }

    const auto w = 1.0f / (d1 + d2);
    const auto w1 = w * d2;
    const auto w2 = w * d1;

    const auto t = clamp(w1 * t1 + w2 * t2, 0.0f, 1.0f);
    const auto p = t * u + v1;

    smooth_curve.vertices[i].position = p;
    smooth_curve.vertices[i].t = t;
  }

  // Generate points
  point_selection.vertices.clear();
  for (auto& v : smooth_curve.vertices) {
    const auto vid1 = v.edge[0];
    const auto vid2 = v.edge[1];
    const auto n1 = mesh.vertices[vid1].normal;
    const auto n2 = mesh.vertices[vid2].normal;
    const auto n = normalize((n2 - n1) * v.t + n1);
    point_selection.vertices.push_back({v.position, n});
  }
  point_selection.update();
}

void viewer::compute_curve_curvature() {}

}  // namespace viewer
