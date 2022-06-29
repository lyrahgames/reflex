#pragma once
#include <libviewer/scene.hpp>
//
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

namespace viewer {

struct loader {
  static void transform(const aiMesh* raw, mesh& mesh) {
    // Get all the vertices.
    mesh.vertices.resize(raw->mNumVertices);
    for (size_t i = 0; i < raw->mNumVertices; i++) {
      mesh.vertices[i].position = {raw->mVertices[i].x,  //
                                   raw->mVertices[i].y,  //
                                   raw->mVertices[i].z};
      mesh.vertices[i].normal = {raw->mNormals[i].x,  //
                                 raw->mNormals[i].y,  //
                                 raw->mNormals[i].z};
    }
    // Get all the faces.
    mesh.faces.resize(raw->mNumFaces);
    for (size_t i = 0; i < raw->mNumFaces; i++)
      for (size_t j = 0; j < raw->mFaces[i].mNumIndices; j++)
        mesh.faces[i][j] = raw->mFaces[i].mIndices[j];
  }

  static void transform(const aiScene* raw, scene& scene) {
    scene.meshes.resize(raw->mNumMeshes);
    for (size_t i = 0; i < raw->mNumMeshes; ++i)
      transform(raw->mMeshes[i], scene.meshes[i]);
  }

  static void load(czstring file_path, scene& scene) {
    Assimp::Importer importer{};
    const auto post_processing =
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals;
    const auto raw = importer.ReadFile(file_path, post_processing);

    if (!raw || raw->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !raw->mRootNode)
      throw runtime_error(string("Failed to load model from given file '") +
                          file_path + "'.");

    transform(raw, scene);
  }
};

}  // namespace viewer
