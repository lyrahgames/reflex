#include <libviewer/viewer.hpp>
//
#include <libviewer/loader.hpp>
//
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace viewer {

viewer::viewer(int w, int h) : screen_width(w), screen_height(h) {
  // To initialize the viewport and matrices,
  // window has to be resized at least once.
  resize();

  // Setup for OpenGL
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glPointSize(3.0f);
  glLineWidth(2.5f);
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

void viewer::update() {
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }

  if (line_read.available()) {
    const auto line = line_read.get();
    cout << line << endl;
    line_read = async_line_read();
  }

  const auto new_time = clock::now();
  const auto dt = duration<float>(new_time - time).count();
  time = new_time;

  scene.animate(dt);
}

void viewer::render() {
  // Clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  scene.render(shader);
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

  shader.bind();
  shader  //
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
  const auto shift = pixels.x * cam.right() + pixels.y * cam.up();
  const auto scale = 1.3f * cam.pixel_size() * radius;
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
  radius = 0.5f * length(aabb_max - aabb_min) *
           (1.0f / tan(0.5f * cam.vfov() * pi / 180.0f));
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
}

}  // namespace viewer
