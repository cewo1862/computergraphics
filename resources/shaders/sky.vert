#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 in_Color;

out vec4 gl_Position;
out vec2 pass_TexCoord;

void main(void)
{
	pass_TexCoord = in_TexCoord;
 	gl_Position = (ProjectionMatrix  * ModelMatrix) * vec4(in_Position, 1.0);
}
