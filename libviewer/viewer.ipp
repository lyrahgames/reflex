#include <libviewer/viewer.hpp>
//
#include <libviewer/loader.hpp>

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

  const auto new_time = clock::now();
  const auto t = duration<float>(new_time - time).count();

  // Compute and set MVP matrix in shader.
  auto model_matrix = glm::mat4{1.0f};
  const auto axis = glm::normalize(glm::vec3(1, 1, 1));
  model_matrix = rotate(model_matrix, 0.1f * t, axis);
  shader.set("model", model_matrix);
}

void viewer::render() {
  // Clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader.bind();
  device_scene.render();
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
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix());
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
  loader::load(file_path, scene);
  fit_view();
  device_scene.load(scene);
}

}  // namespace viewer
