#version 430 core

out vec4 FragColor;

in vec3 NormalVertex;
in vec3 FragmentPosition;

uniform vec3 cameraPosition;
uniform samplerCube skybox;

const float ratio = 1.00 / 1.52;

void main()
{
    vec3 I = normalize(FragmentPosition - cameraPosition);
    vec3 R = refract(I, normalize(NormalVertex), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
