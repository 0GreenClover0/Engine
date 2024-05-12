#version 430 core

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

struct PointLight
{
    vec3 diffuse;
};

const int MAX_POINT_LIGHTS = 1;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

void main()
{
    FragColor = vec4(material.color, 1.0) * vec4(pointLights[0].diffuse, 1.0) * texture(material.texture_diffuse1, fs_in.TextureCoordinatesGeometry);
}
