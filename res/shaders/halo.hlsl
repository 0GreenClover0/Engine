cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
};

cbuffer camera_buffer : register(b2)
{
    float3 camera_pos;
}

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

float4 ps_main(vs_output input) : SV_TARGET
{
    float4 tex_color = obj_texture.Sample(obj_sampler_state, input.uv);

    float3 view_dir = normalize(camera_pos.xyz - input.world_pos.xyz); 

    float normal_dot_view = dot(input.world_normal, view_dir);

    float halo_strength = pow(1.2 - abs(normal_dot_view), 3.0);

    float4 halo_color = float4(1.0, 0.0, 0.0, 1.0) * halo_strength;

    return tex_color + halo_color;
}
