cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
};

struct VS_Input
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 worldNormal : NORMAL;
    float3 worldPos : POSITION;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(projection_view_model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.worldNormal = mul((float3x3) world, input.normal); // Normal in world space
    output.worldPos = mul(world, float4(input.pos, 1.0f)).xyz; // Position in world space
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    // Sample the texture at the given UV coordinates
    float4 texColor = ObjTexture.Sample(ObjSamplerState, input.UV);

    // Calculate view direction
    float3 viewDir = normalize(-input.worldPos); // Assuming camera at origin or using a uniform to pass camera position

    // Calculate the dot product between the normal vector and the view direction
    float NdotV = dot(input.worldNormal, viewDir);

    // Create a halo effect based on the angle between the view vector and the surface normal
    float haloStrength = pow(1.0 - abs(NdotV), 10.0); // Sharpen the effect and control the fall-off

    // Add the halo color (white) to the texture color based on the halo strength
    float4 haloColor = float4(1.0, 1.0, 1.0, 1.0) * haloStrength;

    // Combine original texture color with halo effect
    return texColor + haloColor;
}
