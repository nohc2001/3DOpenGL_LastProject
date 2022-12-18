#version 330 core
out vec4 color;

uniform sampler2D Image;
in vec2 texCoords;

void main()
{
    vec4 inscolor = vec4(vec3(texture(Image, texCoords)), 1);
    if ((inscolor.r < 0.1 && inscolor.g < 0.1) && inscolor.b < 0.1) {
        inscolor = vec4(0, 0, 0, 0);
    }
    color = inscolor;
}