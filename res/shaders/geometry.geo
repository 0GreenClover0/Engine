#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 256) out;

in VS_OUT
{
    vec4 GlPosition;
    vec2 TextureCoordinatesVertex;
} gs_in[];

out GS_OUT
{
    vec2 TextureCoordinatesGeometry;
} gs_out;

uniform float radiusMultiplier = 3.0f;
uniform mat4 PVM;

uniform int sector_count = 6;
uniform int stack_count = 7;

const int max_sector_count = 6;
const int max_stack_count = 7;

const float PI = 3.14159265359;

const int NUM_SPHERE_VERTICES = (2 + (max_stack_count - 1) * (max_sector_count + 1));

void main()
{
    float radius = 7.0 * radiusMultiplier;
    vec4 positions[NUM_SPHERE_VERTICES];
    vec2 texture_coordinates[NUM_SPHERE_VERTICES];

    float phiStep = PI / float(stack_count);
    float thetaStep = 2.0 * PI / float(sector_count);

    uint currentIndex = 1;
    for (uint i = 0; i <= stack_count; ++i)
    {
        float phi = float(i) * phiStep;
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (uint j = 0; j <= sector_count; ++j)
        {

            float i_segment = float(i) / float(stack_count);
            float j_segment = float(j) / float(sector_count);
            texture_coordinates[currentIndex] = vec2(i_segment, j_segment);

            float theta = float(j) * thetaStep;

            vec4 v = gs_in[0].GlPosition;

            v.x += radius * sinPhi * cos(theta);
            v.y += radius * cosPhi;
            v.z += radius * sinPhi * sin(theta);

            positions[currentIndex] = PVM * v;
            ++currentIndex;
        }
    }

    // Generate sphere triangles
    for (uint k = 1; k <= sector_count; ++k)
    {
        gl_Position = positions[0];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[0];
        EmitVertex();

        gl_Position = positions[k + 1];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[k + 1];
        EmitVertex();

        gl_Position = positions[k];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[k];
        EmitVertex();

        EndPrimitive();
    }

    uint baseIndex = 1;
    uint ringVertexCount = sector_count + 1;
    for (uint n = 0; n < stack_count - 2; ++n)
    {
        for (uint m = 0; m <= sector_count; ++m)
        {
            gl_Position = positions[n * ringVertexCount + m];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[n * ringVertexCount + m];
            EmitVertex();

            gl_Position = positions[n * ringVertexCount + m + 1];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[n * ringVertexCount + m + 1];
            EmitVertex();

            gl_Position = positions[(n + 1) * ringVertexCount + m];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[(n + 1) * ringVertexCount + m];
            EmitVertex();

            EndPrimitive();

            gl_Position = positions[(n + 1) * ringVertexCount + m];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[(n + 1) * ringVertexCount + m];
            EmitVertex();

            gl_Position = positions[n * ringVertexCount + m + 1];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[n * ringVertexCount + m + 1];
            EmitVertex();

            gl_Position = positions[(n + 1) * ringVertexCount + m + 1];
            gs_out.TextureCoordinatesGeometry = texture_coordinates[(n + 1) * ringVertexCount + m + 1];
            EmitVertex();

            EndPrimitive();
        }
    }

    const uint southPoleIndex = NUM_SPHERE_VERTICES - 1;

    uint offset = southPoleIndex - ringVertexCount;
    for (uint l = 0; l < sector_count; ++l)
    {
        gl_Position = positions[southPoleIndex];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[southPoleIndex];
        EmitVertex();

        gl_Position = positions[offset + l];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[offset + l];
        EmitVertex();

        gl_Position = positions[offset + l + 1];
        gs_out.TextureCoordinatesGeometry = texture_coordinates[offset + l + 1];
        EmitVertex();

        EndPrimitive();
    }
}
