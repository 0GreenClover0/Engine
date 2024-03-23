cbuffer mycBuffer : register(b0)
{
    float4x4 projection_view;
};

struct VS_Input
{
    float3 pos: POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = mul(projection_view, float4(input.pos.xyz, 1.0f));

    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
