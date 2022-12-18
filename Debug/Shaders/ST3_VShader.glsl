#version 330 core

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in vec3 in_Normal;

uniform mat4 transform;
uniform mat4 cameraTr;
uniform mat4 viewTr;
uniform int bWireframe;

out vec3 out_Color;
out vec2 TexCoord;
out vec3 fNormal;
out vec3 FragPos;

void main(void) {
	vec4 pos = transform * vec4(in_Pos, 1.0);
	pos = cameraTr * pos;
	pos = viewTr * pos;
	gl_Position = pos;

	if (bWireframe == 1) {
		out_Color = vec3(1, 1, 1);
	}
	else {
		out_Color = in_Color;
	}

	TexCoord = in_TexCoord;
	fNormal = mat3(transform) * in_Normal;
	FragPos = vec3(transform * vec4(in_Pos, 1.0));
}