cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
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
    output.pos = mul(projection_view_model,float4(input.pos.xyz, 1.0f));
    output.normal = input.normal;
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float bias = 0.01f;
    float test_value = 0.25f;

    float4 color = ObjTexture.Sample(ObjSamplerState, input.UV);
    clip(color.a - bias);

    if (color.a > bias)
    {
        color.a = test_value;
    }

    return color;
}
