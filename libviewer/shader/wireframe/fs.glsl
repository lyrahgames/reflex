#version 330 core

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  sampler2D texture;
};
uniform Material material;

in vec3 pos;
in vec3 nor;
in vec2 tuv;
noperspective in vec3 edge_distance;

layout (location = 0) out vec4 frag_color;

void main(){
  // Compute distance from edges.
  float d = min(edge_distance.x, edge_distance.y);
  d = min(d, edge_distance.z);
  float line_width = 0.8;
  //   vec4 line_color = vec4(0.8, 0.5, 0.0, 1.0);
  vec4 line_color = texture(material.texture, tuv);
  float mix_value = smoothstep(line_width - 1, line_width + 1, d);
  // Compute viewer shading.
  float ambient = 0.5;
  float diffuse = 0.5 * abs(normalize(nor).z);
  vec4 light_color = vec4(vec3(diffuse + ambient), 1.0);
  // Mix both color values.
  frag_color = mix(line_color, light_color, mix_value);
  //   if (mix_value > 0.9) discard;
  //   frag_color = (1 - mix_value) * line_color;
}
