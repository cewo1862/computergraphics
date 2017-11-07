#version 150

in  vec3 tmp_Color;
out vec4 out_Color;

void main() {
  out_Color = vec4((tmp_Color), 1.0);
}
