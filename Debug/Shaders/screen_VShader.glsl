#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 TexCoords;
uniform vec2 ScreenSiz;
uniform vec2 Mov;

out vec2 texCoords;
void main()
{
    vec3 mpos = vec3(aPos.x + Mov.x, aPos.y + Mov.y, 0);
    float x = (2 * (mpos.x / ScreenSiz.x) + 1) - 2;
    float y = 2 - (2 * (mpos.y / ScreenSiz.y) + 1);
    gl_Position = vec4(x, y, -0.9999, 1.0);
    texCoords = TexCoords;
}