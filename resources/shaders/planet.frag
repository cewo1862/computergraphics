#version 150

in vec3 pass_Normal;
in vec3 pass_Color;
in vec4 gl_FragCoord;
in vec3 sun_direction;
in vec3 view_direction;
in vec2 pass_TexCoord;

uniform sampler2D ColorTex;

out vec4 out_Color;

vec3 light_color = vec3(1.0,1.0,1.0);
float light_shininess = 50.0;

vec3 color = texture(ColorTex, pass_TexCoord).xyz;

vec3 ambient()
{
	return color * light_color;
}

vec3 diffuse(in vec3 object_Normal, in vec3 sun_Normal){
	float diffuseTerm = clamp(dot(object_Normal,sun_Normal),0,1);
	return (color * light_color * diffuseTerm);
}

vec3 specular(in vec3 object_Normal, in vec3 view_Normal, in vec3 sun_Normal){
	float specularTerm = 0;

	if(dot(object_Normal, sun_Normal) > 0){
		vec3 middle_Normal = normalize(sun_Normal + view_Normal);
		specularTerm = pow(dot(object_Normal, middle_Normal), light_shininess);
	}
	return vec3(1.0,1.0,1.0) * light_color * specularTerm;
}

void main() {
  	
  	vec3 object_Normal = normalize(pass_Normal);
  	vec3 view_Normal = normalize(view_direction);
  	vec3 sun_Normal = normalize(sun_direction);

  	vec3 amb_value = ambient();
  	vec3 dif_value = diffuse(object_Normal,sun_Normal);
  	vec3 spec_value = specular(object_Normal,view_Normal,sun_Normal);

  	out_Color = vec4((amb_value+dif_value+spec_value),1.0);
}
