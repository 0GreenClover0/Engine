cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
};

struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(projection_view_model, float4(input.pos.xyz, 1.0f));
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
