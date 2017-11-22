#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 in_Color;

out vec3 pass_Normal;
out vec3 pass_Color;
out vec4 gl_Position;
out vec3 sun_direction;
out vec3 view_direction;

void main(void)
{
	vec4 tmp_vec;
	tmp_vec = ViewMatrix * ModelMatrix * vec4(in_Position, 1.0);
	vec4 sun;
	sun = ViewMatrix * vec4(0.0,0.0,0.0,1.0);
	sun_direction = (sun.xyz-tmp_vec.xyz);
	view_direction = (vec3(0.0,0.0,0.0)-tmp_vec.xyz);
	pass_Color = in_Color;
	pass_Normal = ((NormalMatrix * vec4(in_Normal, 0.0)).xyz);
 	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}
