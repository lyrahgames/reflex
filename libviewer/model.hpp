#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

struct model {
  struct vertex {
    vec3 position;
    vec3 normal;
  };

  using face = array<uint32_t, 3>;

  void setup(const shader_program& shader) {
    // Use a vertex array to be able to reference the vertex buffer and
    // the vertex attribute arrays of the triangle with one single variable.
    // glGenVertexArrays(1, &handle);
    // glBindVertexArray(handle);
    handle.bind();

    vertex_data.bind();
    face_data.bind();

    // Set the data layout of the position and colors
    // with vertex attribute pointers.
    {
      const auto location = glGetAttribLocation(shader, "p");
      glEnableVertexAttribArray(location);
      glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, position));
    }
    {
      const auto location = glGetAttribLocation(shader, "n");
      glEnableVertexAttribArray(location);
      glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, normal));
    }
  }

  void update() {
    // Generate and bind the buffer which shall contain the triangle data.
    // glGenBuffers(1, &vertex_data);
    // glBindBuffer(GL_ARRAY_BUFFER, vertex_data);
    vertex_data.bind();
    // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
                 vertices.data(), GL_STATIC_DRAW);

    // glGenBuffers(1, &face_data);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_data);
    face_data.bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(faces[0]),
                 faces.data(), GL_STATIC_DRAW);
  }

  void render() {
    // glBindVertexArray(handle);
    handle.bind();
    // glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
  }

  vector<vertex> vertices{};
  vector<face> faces{};

  // GLuint handle;
  // GLuint vertex_data;
  // GLuint face_data;

  vertex_array handle;
  vertex_buffer vertex_data;
  element_buffer face_data;
};

}  // namespace viewer
