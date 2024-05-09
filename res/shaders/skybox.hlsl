TextureCube skybox_texture : register(t0);
SamplerState obj_sampler_state : register(s0);

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
    float4x4 light_projection_view_model;
};

struct VS_Input
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    //Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
    output.pos = mul(projection_view_model, float4(input.pos, 1.0f)).xyww;

    output.texCoord = input.pos;

    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return skybox_texture.Sample(obj_sampler_state, input.texCoord);
}
