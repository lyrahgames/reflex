#version 420 core

struct Camera {
  mat4 projection;
  mat4 view;
  mat4 viewport;
};

uniform Camera camera;

layout (std140, binding = 0)
uniform uniforms {
  mat4 projection;
  mat4 view;
};

uniform mat4 model;
uniform mat3 normal_matrix;

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 uv;

out vertex_data {
  vec3 normal;
  vec2 texuv;
} v;

void main(){
  gl_Position = projection * view * model * vec4(p, 1.0);
  v.normal = vec3(camera.view * vec4(normal_matrix * n, 0.0));
  v.texuv = uv;
}
