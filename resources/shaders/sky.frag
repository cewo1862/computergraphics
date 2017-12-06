#version 150

in vec2 pass_TexCoord;

uniform sampler2D ColorTex;

out vec4 out_Color;

void main() {
out_Color = texture(ColorTex, pass_TexCoord);
}
