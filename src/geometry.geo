#version 400 core
layout (points) in;
layout (triangle_strip, max_vertices = 255) out;

in VS_OUT
{
    vec4 GlPosition;
    vec2 TexCoord;
} gs_in[];

out GS_OUT
{
    vec2 TexCoord;
} gs_out;

const float radius = 10.0f;
const int sector_count = 6;
const int stack_count = 7;
const float PI = 3.14159265359;

void main()
{
    //for (int i = 0; i < 3; i++)
    //{
        //gl_Position = gs_in[i].GlPosition;
        //gs_out.TexCoord = gs_in[i].TexCoord;
        //EmitVertex();
    //}

    //EndPrimitive();
}