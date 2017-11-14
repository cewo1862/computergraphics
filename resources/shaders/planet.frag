#version 150

in vec3 pass_Color;
in vec4 gl_FragCoord;

void main() {
  out_Color = vec4(abs(normalize(pass_Color)), 1.0);
}
