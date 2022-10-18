#version 330 core

struct Camera {
  mat4 projection;
  mat4 view;
  mat4 viewport;
};

uniform Camera camera;
uniform mat4 model;
uniform mat3 normal_matrix;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  sampler2D texture;
};

uniform Material material;

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 uv;

flat out vec3 color;

void main(){
  gl_Position = camera.projection * camera.view * model * vec4(p, 1.0);

  vec3 normal = vec3(camera.view * vec4(normal_matrix * n, 0.0));

  vec3 light_color = vec3(0.3, 0.3, 0.3);

  vec3 view_dir = vec3(0.0, 0.0, 1.0);
  vec3 light_dir = view_dir;
  vec3 reflect_dir = reflect(-light_dir, normal);

  color = material.ambient;

  float d = max(dot(light_dir, normal), 0.0);
  color += d * material.diffuse;

  float s = pow(max(dot(reflect_dir, normal), 0.0), material.shininess);
  color += s * material.specular;

  vec3 tex = vec3(texture(material.texture, uv));
  color *= tex * light_color;
}
