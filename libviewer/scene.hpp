#pragma once
#include <libviewer/model.hpp>

namespace viewer {

struct scene {
  // void add(const model& mesh) { meshes.push_back(mesh); }
  // void add(model&& mesh) { meshes.push_back(move(mesh)); }

  void setup(const shader_program& shader) {
    for (auto& mesh : meshes) mesh.setup(shader);
  }

  void update() {
    for (auto& mesh : meshes) mesh.update();
  }

  void render() {
    for (auto& mesh : meshes) mesh.render();
  }

  vector<model> meshes{};
};

}  // namespace viewer
