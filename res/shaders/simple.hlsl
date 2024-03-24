cbuffer mycBuffer : register(b0)
{
    float4x4 projection_view;
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
    float2 UV : TEXCOORD;
    float3 normal : NORMAL;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(projection_view, float4(input.pos.xyz, 1.0f));
    output.normal = mul(input.normal,input.pos);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return ObjTexture.Sample(ObjSamplerState, input.UV);
}
