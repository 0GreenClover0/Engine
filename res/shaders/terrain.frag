#version 430 core

in float Height;

out vec4 FragColor;

void main()
{
    float h = (Height + 16.0) / 32.0;
    FragColor = vec4(h, h, h, 1.0);
}