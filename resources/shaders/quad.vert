#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform int vert_switch;
uniform int hori_switch;

out vec4 gl_Position;
out vec2 pass_TexCoord;

void main(void)
{
vec2 tmp_TexCoord = in_TexCoord;

	if(vert_switch == 1){
		if(in_TexCoord.y == 1.0){
		tmp_TexCoord.y = 0.0;
		}
		else if(in_TexCoord.y == 0.0){
		tmp_TexCoord.y = 1.0;
		}
	}
	if(hori_switch == 1){
		if(in_TexCoord.x == 1.0){
		tmp_TexCoord.x = 0.0;
		}
		else if(in_TexCoord.x == 0.0){
		tmp_TexCoord.x = 1.0;
		}	
	}
	pass_TexCoord = tmp_TexCoord;
 	gl_Position = vec4(in_Position,1.0);
}
