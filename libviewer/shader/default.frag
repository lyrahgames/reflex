#version 330 core

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  sampler2D texture;
};

uniform Material material;

in vertex_data {
  vec3 normal;
  vec2 texuv;
} v;

layout (location = 0) out vec4 frag_color;

void main(){
  vec3 n = normalize(v.normal);

  vec3 light_color = vec3(0.3, 0.3, 0.3);

  vec3 view_dir = vec3(0.0, 0.0, 1.0);
  vec3 light_dir = view_dir;
  vec3 reflect_dir = reflect(-light_dir, n);

  vec3 color = material.ambient;

  float d = max(dot(light_dir, n), 0.0);
  color += d * material.diffuse;

  float s = pow(max(dot(reflect_dir, n), 0.0), material.shininess);
  color += s * material.specular;

  vec3 tex = vec3(texture(material.texture, v.texuv));
  color *= tex * light_color;

  frag_color = vec4(color, 1.0);
}
