#version 330 core

struct Camera {
  mat4 projection;
  mat4 view;
  mat4 viewport;
};
uniform Camera camera;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 position[];
in vec3 normal[];
in vec2 texuv[];

out vec3 pos;
out vec3 nor;
out vec2 tuv;
noperspective out vec3 edge_distance;

void main(){
  vec3 p0 = vec3(camera.viewport * (gl_in[0].gl_Position /
                             gl_in[0].gl_Position.w));
  vec3 p1 = vec3(camera.viewport * (gl_in[1].gl_Position /
                             gl_in[1].gl_Position.w));
  vec3 p2 = vec3(camera.viewport * (gl_in[2].gl_Position /
                             gl_in[2].gl_Position.w));

  float a = length(p1 - p2);
  float b = length(p2 - p0);
  float c = length(p1 - p0);

  float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
  float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));

  float ha = abs(c * sin(beta));
  float hb = abs(c * sin(alpha));
  float hc = abs(b * sin(alpha));

  edge_distance = vec3(ha, 0, 0);
  nor = normal[0];
  pos = position[0];
  tuv = texuv[0];
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  edge_distance = vec3(0, hb, 0);
  nor = normal[1];
  pos = position[1];
  tuv = texuv[1];
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  edge_distance = vec3(0, 0, hc);
  nor = normal[2];
  pos = position[2];
  tuv = texuv[2];
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();

  EndPrimitive();
}
