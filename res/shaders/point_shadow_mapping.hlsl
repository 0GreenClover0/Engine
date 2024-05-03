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

cbuffer depth_constants : register(b1)
{
    float3 light_pos;
    float far_plane;
}

struct VS_Output
{
    float4 pixel_pos : SV_POSITION; 
    float4 world_pos : POSITION;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.pixel_pos = mul(projection_view_model, float4(input.pos,1.0f));
    return output;
}

float ps_main(VS_Output input) : SV_Depth
{
    float light_distance = length(input.world_pos.xyz - light_pos);
    light_distance = light_distance / far_plane;
    return light_distance;
}
