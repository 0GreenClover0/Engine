#version 430 core

layout (location = 0) in vec3 PositionInput;

out vec3 TextureCoordinatesVertex;

uniform mat4 PVnoTranslation;

void main()
{
    TextureCoordinatesVertex = PositionInput;
    vec4 position = PVnoTranslation * vec4(PositionInput, 1.0);
    gl_Position = position.xyww;
}
