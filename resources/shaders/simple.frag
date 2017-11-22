#version 150

in  vec3 pass_Normal;
out vec4 out_Color;

void main() {
	vec4 tmp_vec
	tmp_vec = ViewMatrix * ModelMatrix * vec4(in_Position, 1.0);
  	out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
  	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}
