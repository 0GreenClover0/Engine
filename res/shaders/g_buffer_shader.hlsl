struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_Output
{
    float4 pixel_pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 world_pos : POSITION;
    float2 UV : TEXCOORD;
};

struct PS_Output
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 diffuse : SV_Target2;
};

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
    float4x4 projection_view;
};

Texture2D obj_texture : register(t0);
SamplerState obj_sampler_state : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.normal = mul((float3x3)model, input.normal);
    output.pixel_pos = mul(projection_view_model, float4(input.pos, 1.0f));
    return output;
}

PS_Output ps_main(VS_Output input)
{
    PS_Output output;
    output.diffuse = obj_texture.Sample(obj_sampler_state, input.UV);
    output.position.xyz = input.world_pos;
    output.normal.xyz = input.normal;
    output.normal.a = 1.0f;
    output.position.a = 1.0f;
    
    return output;
}
