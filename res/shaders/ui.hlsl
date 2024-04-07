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
};

cbuffer VS_Buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
    float4x4 projection;
};

cbuffer FS_Buffer : register(b1)
{
    float red;
    float green;
    float blue;
};

Texture2D UITexture;
SamplerState UISampler;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(world, float4(input.pos.xy, 0.0, 1.0));
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return UITexture.Sample(UISampler, input.UV);
}
