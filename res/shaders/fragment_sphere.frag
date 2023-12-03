#version 400 core

out vec4 FragColor;

in GS_OUT
{
    vec2 TextureCoordinatesGeometry;
} fs_in;

struct Material
{
    vec3 color;
};

uniform Material material;

struct Light
{
    vec3 diffuse;
};

uniform Light light;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;

void main()
{
    FragColor = material.color * light.diffuse * texture(texture_diffuse1, fs_in.TextureCoordinatesGeometry);
}