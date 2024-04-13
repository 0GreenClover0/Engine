struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD0;
};

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
};

struct VS_Output
{
    float4 pixel_pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.pixel_pos = mul(projection_view_model, float4(input.pos, 1.0f));
    output.UV = output.pixel_pos;
    return output;
}

// This function is discarded anyway, but it safer to at least write a dummy one
float ps_main(VS_Output input) : SV_Target
{
    return 1.0f;
}
