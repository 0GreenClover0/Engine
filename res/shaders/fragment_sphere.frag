#version 400 core

out vec4 FragColor;

in GS_OUT
{
    vec2 TextureCoordinatesGeometry;
} fs_in;

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
    FragColor = material.color * light.diffuse * texture(material.texture_diffuse1, fs_in.TextureCoordinatesGeometry);
}