#version 430 core

layout (location = 0) in vec3 PositionInput;
layout (location = 1) in vec3 NormalInput;
layout (location = 2) in vec2 TextureCoordinatesInput;

out vec2 TextureCoordinatesVertex;
out vec3 FragmentPosition;
out vec3 NormalVertex;

uniform mat4 PVM;
uniform mat4 model;

void main()
{
    gl_Position = PVM * vec4(PositionInput, 1.0);
    FragmentPosition = vec3(model * vec4(PositionInput, 1.0));

    // TODO: Do this on the CPU
    NormalVertex = mat3(transpose(inverse(model))) * NormalInput;
    TextureCoordinatesVertex = TextureCoordinatesInput;
}
