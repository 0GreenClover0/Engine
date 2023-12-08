#version 430 core

struct Material
{
    vec3 color;
};

uniform Material material;

out vec4 FragColor;

void main()
{
    FragColor = vec4(material.color, 1.0);
}
