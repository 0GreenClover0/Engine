#include "lighting_calculations.hlsl"

struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos: SV_Position;
    float2 UV : TEXCOORD0;
};

Texture2D pos_tex : register(t10);
Texture2D normal_tex : register(t11);
Texture2D diffuse_tex : register(t12);
Texture2D ambient_occlusion_tex : register(t14);
Texture2D fog_tex : register(t16);

SamplerState obj_sampler_state : register(s0);
SamplerState repeat_sampler : register(s2);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 pos = pos_tex.Sample(obj_sampler_state, input.UV);
    float4 normal = normalize(normal_tex.Sample(obj_sampler_state, input.UV));
    float4 diffuse = diffuse_tex.Sample(obj_sampler_state, input.UV);
    float ambient_occlusion = ambient_occlusion_tex.Sample(obj_sampler_state, input.UV).r;

    if (diffuse.r == 0.0f && diffuse.g == 0.0f && diffuse.b == 0.0f)
    {
        discard;
    }

    float3 view_dir = normalize(camera_pos.xyz - pos.xyz);

    result.xyz += calculate_directional_light(directional_light, normal.xyz, view_dir, diffuse.xyz, pos.xyz, true, ambient_occlusion);
    float fog_value = 1.0f; 
    if (is_fog_rendered)
    {
        fog_value = fog_tex.Sample(repeat_sampler, input.UV + time_ps / 100.0f).r;
        result += 0.2f * fog_value;
    }

    for (int point_light_index = 0; point_light_index < number_of_point_lights; point_light_index++)
    {
        result.xyz += calculate_point_light(point_lights[point_light_index],normal.xyz, pos.xyz, view_dir, diffuse.xyz, point_light_index, true);
        result.xyz += calculate_scatter(point_lights[point_light_index], pos) * fog_value;
    }

    for (int spot_light_index = 0; spot_light_index < number_of_spot_lights; spot_light_index++)
    {
        result.xyz += calculate_spot_light(spot_lights[spot_light_index], normal.xyz, pos.xyz, view_dir, diffuse.xyz, spot_light_index, true);
        result.xyz += calculate_scatter(spot_lights[spot_light_index], pos, spot_light_index) * fog_value;
    }

    result.xyz = exposure_tonemapping(result.xyz, 1.1f);
    return gamma_correction(result.xyz);
}
