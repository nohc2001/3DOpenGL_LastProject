#version 330 core
layout(location = 0) in vec3 aPos;

uniform vec2 ScreenSiz;

void main()
{
    float x = (2 * (aPos.x / ScreenSiz.x) + 1) - 2;
    float y = 2 - (2 * (aPos.y / ScreenSiz.y) + 1);
    gl_Position = vec4(x, y, -1, 1.0);
}