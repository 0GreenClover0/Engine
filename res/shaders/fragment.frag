#version 400 core

out vec4 FragColor;

in vec2 TextureCoordinatesVertex;

struct Material
{
    vec3 color;

    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_specular3;
};

uniform Material material;

struct Light
{
    vec3 diffuse;
};

uniform Light light;

void main()
{
    FragColor = vec4(material.color, 1.0) * vec4(light.diffuse, 1.0) * texture(material.texture_diffuse1, TextureCoordinatesVertex);
}