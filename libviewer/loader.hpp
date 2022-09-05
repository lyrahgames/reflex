#pragma once
#include <unordered_map>
//
#include <libviewer/scene.hpp>
//
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

namespace viewer {

struct loader {
  void transform(const aiScene* raw_scene,
                 const aiMesh* raw_mesh,
                 scene& scene,
                 mesh& mesh) {
    // Get all the vertices.
    mesh.vertices.resize(raw_mesh->mNumVertices);
    for (size_t i = 0; i < raw_mesh->mNumVertices; i++) {
      mesh.vertices[i].position = {raw_mesh->mVertices[i].x,  //
                                   raw_mesh->mVertices[i].y,  //
                                   raw_mesh->mVertices[i].z};
      mesh.vertices[i].normal = {raw_mesh->mNormals[i].x,  //
                                 raw_mesh->mNormals[i].y,  //
                                 raw_mesh->mNormals[i].z};

      if (raw_mesh->mTextureCoords[0])
        mesh.vertices[i].uv = {raw_mesh->mTextureCoords[0][i].x,
                               raw_mesh->mTextureCoords[0][i].y};
      else
        mesh.vertices[i].uv = {};
    }

    // Get all the faces.
    mesh.faces.resize(raw_mesh->mNumFaces);
    for (size_t i = 0; i < raw_mesh->mNumFaces; i++)
      for (size_t j = 0; j < raw_mesh->mFaces[i].mNumIndices; j++)
        mesh.faces[i][j] = raw_mesh->mFaces[i].mIndices[j];

    // Get the materials and textures.
    if (raw_mesh->mMaterialIndex >= 0) {
      auto raw_material = raw_scene->mMaterials[raw_mesh->mMaterialIndex];
      mesh.material_id = scene.materials.size();

      scene.materials.push_back({raw_material->GetName().C_Str()});

      aiColor3D color(0.f, 0.f, 0.f);

      raw_material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
      scene.materials.back().diffuse = vec3(color.r, color.g, color.b);

      raw_material->Get(AI_MATKEY_COLOR_AMBIENT, color);
      scene.materials.back().ambient = vec3(color.r, color.g, color.b);

      raw_material->Get(AI_MATKEY_COLOR_SPECULAR, color);
      scene.materials.back().specular = vec3(color.r, color.g, color.b);

      raw_material->Get(AI_MATKEY_SHININESS, scene.materials.back().shininess);

      // auto diffuse_tex_count = material->GetTextureCount(aiTextureType_DIFFUSE);
      // for (size_t texid = 0; texid < diffuse_tex_count; ++texid) {
      if (raw_material->GetTextureCount(aiTextureType_DIFFUSE)) {
        aiString path;
        auto texture =
            raw_material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        scene.materials.back().texture_path = directory / path.C_Str();
      }
      // }
    }
  }

  void transform(const aiScene* raw_scene, scene& scene) {
    scene.meshes.resize(raw_scene->mNumMeshes);
    for (size_t i = 0; i < raw_scene->mNumMeshes; ++i) {
      transform(raw_scene, raw_scene->mMeshes[i], scene, scene.meshes[i]);

      // cout << "Mesh " << i << ":\n" << endl;
    }
  }

  void load(czstring file_path, scene& scene) {
    filesystem::path path = file_path;
    path = filesystem::canonical(path);
    directory = path.parent_path();

    cout << path << endl << directory << endl;

    Assimp::Importer importer{};
    const auto post_processing =
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals;
    // | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_GenSmoothNormals;
    const auto raw = importer.ReadFile(file_path, post_processing);

    if (!raw || raw->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !raw->mRootNode)
      throw runtime_error(string("Failed to load model from given file '") +
                          file_path + "'.");

    transform(raw, scene);

    // for (size_t i = 0; i < raw->mNumMaterials; ++i) {
    //   auto material = raw->mMaterials[i];

    //   // aiString name;
    //   // material->Get(AI_MATKEY_NAME, name);

    //   auto name = material->GetName();

    //   aiColor3D diffuse_color;
    //   material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);

    //   cout << "Material " << i << ":\n"
    //        << "  name = " << name.C_Str() << '\n'
    //        << "  diffuse color = (" << diffuse_color.r << ", "
    //        << diffuse_color.g << ", " << diffuse_color.b << ")\n";
    //   // << "  diffuse texture count = " << diffuse_tex_count << '\n'

    //   auto diffuse_tex_count = material->GetTextureCount(aiTextureType_DIFFUSE);
    //   for (size_t texid = 0; texid < diffuse_tex_count; ++texid) {
    //     aiString path;
    //     auto texture =
    //         material->GetTexture(aiTextureType_DIFFUSE, texid, &path);
    //     cout << "  diffuse texture " << texid << ":\n"
    //          << "    path = " << path.C_Str() << '\n';
    //   }

    //   cout << endl;
    // }
  }

  filesystem::path directory;
};

}  // namespace viewer
