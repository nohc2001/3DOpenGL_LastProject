#version 330 core
out vec4 color;

uniform vec3 textColor;

void main()
{
    color = vec4(textColor, 1.0);
}