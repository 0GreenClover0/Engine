#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{
	vec4 GlPosition;
	vec2 TexCoord;
} vs_out;

uniform mat4 transform;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.GlPosition = projection * view * model * vec4(aPos, 1.0);
	vs_out.TexCoord = aTexCoord;
}