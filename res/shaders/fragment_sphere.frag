#version 400 core

out vec4 FragColor;

in GS_OUT
{
    vec2 TextureCoordinatesGeometry;
} fs_in;

uniform vec4 objectColor;
uniform vec4 lightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;

void main()
{
    FragColor = objectColor * lightColor * texture(texture_diffuse1, fs_in.TextureCoordinatesGeometry);
}