#define PI 3.141592f
#define gravity 9.81f

#include "lighting_calculations.hlsl"

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
    float4x4 projection_view;
};

cbuffer time_buffer : register(b1)
{
    Wave waves[15];
    float time;
    int no_of_waves;
}

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
    float3 ndc : TEXCOORD1;
};

struct PositionAndNormal
{
    float3 position;
    float3 normal;
};

Texture2D obj_texture : register(t0);
TextureCube skybox : register(t15);
Texture2D fog_tex : register(t16);

SamplerState obj_sampler_state : register(s0);

// VERTEX SHADER FUNCTIONS
PositionAndNormal calc_gerstner_wave_position_and_normal(float x, float y, float3 position_for_normal)
{
    float3 pos = float3(x, 0.0f, y);
    float3 normal = float3(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < no_of_waves; i++)
    {
        float speed = waves[i].speed;

        // Main variables
        float wave_length = waves[i].wave_length;
        float amplitude = waves[i].amplitude;
        float2 direction = waves[i].direction;

        float frequency = sqrt(gravity * 2.0f * PI / wave_length);
        //float frequency =  2.0f / wave_length;
        float steepness = waves[i].steepness / (frequency * amplitude * no_of_waves);
        float phi = speed * 2.0f / wave_length;

        // Position calculation
        pos.x += steepness * amplitude * direction.x * cos(dot((frequency * direction), float2(x, y)) + phi * time);
        pos.z += steepness * amplitude * direction.y * cos(dot((frequency * direction), float2(x, y)) + phi * time);
        pos.y += amplitude * sin(dot((frequency * direction), float2(x, y)) + phi * time);

        // Normal calculation
        // Variable naming from:
        //https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
        float WA = frequency * amplitude;
        float S_func = sin(frequency * dot(direction, position_for_normal.xz) + phi * time);
        float C_func = cos(frequency * dot(direction, position_for_normal.xz) + phi * time);

        normal.x -= direction.x * WA * C_func;
        normal.z -= direction.y * WA * C_func;
        normal.y -= steepness * WA * S_func;
    }

    PositionAndNormal result;
    result.position = pos;
    result.normal = normal;

    return result;
}

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.world_pos = mul(model, float4(input.pos, 1.0f));

    PositionAndNormal pos_and_normal = calc_gerstner_wave_position_and_normal(output.world_pos.x, output.world_pos.z, input.pos);

    output.normal = mul(pos_and_normal.normal, (float3x3)model);
    output.world_pos = pos_and_normal.position;
    output.UV = input.UV;
    output.pixel_pos = mul(projection_view, float4(output.world_pos, 1.0f));
    output.ndc = output.pixel_pos.xyz / output.pixel_pos.w * 0.5f + 0.5f;
    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float2 screen_UV = float2(input.ndc.x, - input.ndc.y);
    float ratio = 1.0f / 1.25f;
    float3 norm = normalize(input.normal);
    float3 I = normalize(input.world_pos - camera_pos);
    float3 R_refract = refract(I, norm, ratio);
    float3 R_reflect = reflect(I, norm);
    float3 refraction = skybox.Sample(obj_sampler_state, R_refract).rgb;
    float3 reflection = skybox.Sample(obj_sampler_state, R_reflect).rgb;
    
    float3 view_dir = normalize(camera_pos.xyz - input.world_pos.xyz);
    float3 diffuse_texture = obj_texture.Sample(obj_sampler_state, input.UV).rgb;

    float3 result = calculate_directional_light(directional_light, norm, view_dir, diffuse_texture, input.world_pos, false);

    float fog_value = 1.0f; 
    if (is_fog_rendered)
    {
        fog_value = fog_tex.Sample(obj_sampler_state, screen_UV + time_ps / 100.0f).r;
        result += 0.2f * fog_value;
    }

    for (int i = 0; i < number_of_point_lights; i++)
    {
        result += calculate_point_light(point_lights[i], norm, input.world_pos.rgb, view_dir, diffuse_texture, i, false);
        result += calculate_scatter(point_lights[i], float4(input.world_pos, 1.0f)) * fog_value;
    }

    for (int j = 0; j < number_of_spot_lights; j++)
    {
        result += calculate_spot_light(spot_lights[j], norm, input.world_pos, view_dir, diffuse_texture, j, true);
        result += calculate_scatter(spot_lights[j], float4(input.world_pos, 1.0f), j) * fog_value;
    }

    result += reflection * 0.5f;

    float4 return_value = gamma_correction(result);
    return_value.a = 0.8f;
    return return_value;
}
