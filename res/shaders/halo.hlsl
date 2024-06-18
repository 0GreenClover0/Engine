#include "lighting_calculations.hlsl"

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
};

struct vs_input
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct vs_output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 world_normal : NORMAL;
    float3 world_pos : POSITION;
};

Texture2D obj_texture : register(t0);
SamplerState obj_sampler_state : register(s0);

vs_output vs_main(vs_input input)
{
    vs_output output;
    output.pos = mul(projection_view_model, float4(input.pos, 1.0f));
    output.uv = input.uv;
    output.world_normal = mul((float3x3) world, input.normal); 
    output.world_pos = mul(world, float4(input.pos, 1.0f)).xyz;
    return output;
}

float4 glow(float3 color, float3 world_pos, float3 world_normal)
{
    float3 view_dir = normalize(camera_pos.xyz - world_pos.xyz);
    color.xyz = calculate_directional_light(directional_light, world_normal, view_dir, color.xyz, world_pos, true);

    float normal_dot_view = dot(world_normal, view_dir);

    float halo_strength = pow(1.1f - abs(normal_dot_view), 1.5f);

    float3 halo_color = float3(1.0f, 1.0f, 1.0f) * halo_strength;
    return float4((color + halo_color), 1.0f);
}

float4 ps_main(vs_output input) : SV_TARGET
{
    float4 tex_color = obj_texture.Sample(obj_sampler_state, input.uv);

    return glow(tex_color.xyz, input.world_pos, input.world_normal);
}
