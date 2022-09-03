#version 330 core

struct Camera {
  mat4 projection;
  mat4 view;
  mat4 viewport;
};
uniform Camera camera;

uniform mat4 model;

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 uv;

out vec3 position;
out vec3 normal;
out vec2 texuv;

void main(){
  gl_Position = camera.projection * camera.view * model * vec4(p, 1.0);
  position = vec3(camera.view * model * vec4(p, 1.0));
  normal = vec3(camera.view * model * vec4(n, 0.0));
  texuv = uv;
}
