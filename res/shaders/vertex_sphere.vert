#version 430 core

layout (location = 0) in vec3 PositionInput;
layout (location = 1) in vec3 NormalInput;
layout (location = 2) in vec2 TextureCoordinatesInput;

out VS_OUT
{
    vec4 GlPosition;
    vec2 TextureCoordinatesVertex;
} vs_out;

uniform mat4 PVM;

void main()
{
    vs_out.GlPosition = PVM * vec4(PositionInput, 1.0);
    vs_out.TextureCoordinatesVertex = TextureCoordinatesInput;
}
