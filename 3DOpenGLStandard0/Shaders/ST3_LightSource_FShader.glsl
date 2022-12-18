#version 330 core

in vec3 out_Color;
in vec2 TexCoord;
out vec4 color;

struct Material {
	sampler2D diffuse;
};

uniform Material material;

void main(void) {
	vec4 fcol = texture(material.diffuse, TexCoord);
	color = vec4(fcol.xyz, 1.0);
}