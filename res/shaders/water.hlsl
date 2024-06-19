#define PI 3.141592f
#define gravity 9.81f

#include "ssr.hlsl"
#include "ShadingDefines.h"

cbuffer water_buffer : register(b4)
{
    float4 top_color;
    float4 bottom_color;

    float normalmap_scroll_speed0;
    float normalmap_scroll_speed1;

    float normalmap_scale0;
    float normalmap_scale1;

    float combined_amplitude;
    float phong_contribution; // Value between 0.0f and 1.0f
}

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

TextureCube skybox : register(t15);
Texture2D fog_tex : register(t16);
Texture2D water_normal0 : register(t18);
Texture2D water_normal1 : register(t19);

SamplerState clamp_border_sampler : register(s3);

// VERTEX SHADER FUNCTIONS
PositionAndNormal calc_gerstner_wave_position_and_normal(float x, float y, float3 position_for_normal)
{
    float3 pos = float3(x, 0.0f, y);
    float3 normal = float3(0.0f, 1.0f, 0.0f);

    [loop]
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
    result.normal = normalize(normal);
    return result;
}

// PIXEL SHADER functions
float falloff(float3 current_world_position, float3 other_world_position, float threshold = 0.1f, float power = 10.0f)
{
    float distance = length(other_world_position - current_world_position);
    if (abs(distance) < threshold)
    {
        return 1.0f - distance * power;
    }

    return 0.0f;
}

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.world_pos = mul(model, float4(input.pos, 1.0f));

    PositionAndNormal pos_and_normal = calc_gerstner_wave_position_and_normal(output.world_pos.x, output.world_pos.z, input.pos);

    output.normal = normalize(mul((float3x3)model, pos_and_normal.normal));
    output.world_pos = pos_and_normal.position;
    output.UV = input.UV;
    output.pixel_pos = mul(projection_view, float4(output.world_pos, 1.0f));
    output.ndc = output.pixel_pos.xyz / output.pixel_pos.w * 0.5f + 0.5f;
    return output;
}

float4 halo(float3 halo_center, float3 world_pos, float diameter)
{
    // This function makes a 2D halo on water
    float distance = length(world_pos - halo_center);

    float time = time_ps;
    if (time > PI / 2.0f)
    {
        time += PI / 2.0f - time;
    }

    float radius = diameter / 2.0f;
    radius *= 0.55f;

    float _border = radius * tan(time_ps) * tan(time_ps);
    _border = clamp(_border, 0.0f, radius);
    if (distance < _border)
    {
        float strength = -log10(-(distance) / (_border) + 1.0f);
        return float4(0.5f, 0.5f, 1.0f, 1.0f) * strength;
    }
    else if (distance < radius)
    {
        float strength = 1.0f + log10(-(radius - distance) / (radius) + 1.0f);
        return float4(0.5f, 0.5f, 1.0f, 1.0f) * strength;
    }
    else
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    // Calculate mouse_pos halo
    float4 halo_value = 0.0f.xxxx;
    if (light_range > 0.0f)
    {
        halo_value = halo(float3(mouse_pos.x, 0.0f, mouse_pos.y), input.world_pos.xyz, light_range);
        halo_value /= 10.0f;
    }

    // FALLOFF CALCULATION
    float2 UV = float2(input.ndc.x, (-input.ndc.y + 1.0f));
    float3 deferred_world_pos = position_buffer.Sample(clamp_border_sampler, UV).xyz;
    float falloff_value = falloff(input.world_pos, deferred_world_pos);

    // NORMAL MAPPING
    float3 normal1 = water_normal0.Sample(wrap_sampler, (input.UV.xy + time_ps.xx * normalmap_scroll_speed0) * normalmap_scale0) * float4(0.1f.xxx, 1.0f);
    float3 normal2 = water_normal1.Sample(wrap_sampler, (input.UV.xy + time_ps.xx * -normalmap_scroll_speed1) * normalmap_scale1);
    float3 combined_normal = normal_blend(normalize(normal2 * 2.0f - 1.0f), input.normal);
    combined_normal = normal_blend(normalize(normal1 * 2.0f - 1.0f), combined_normal);

    // VIEW SPACE VARIABLES
    float3 view_normal = normalize(mul((float3x3)view, combined_normal));
    // In this case, normal2 has it's B channel almost removed, so it doesn't offset reflections
    float3 view_reflection_normal = normalize(mul((float3x3)view, normalize(input.normal + normal2 * float3(0.3f, 0.3f, 0.05f))));
    float3 view_pos = mul(view, float4(input.world_pos, 1.0f)).xyz;

    // REFRACTION
    float3 refracted_vector = normalize(refract(normalize(view_pos), view_normal, 1.0f/1.52f));
    float4 refraction_coords = ray_cast(refracted_vector, view_pos, false);
    float4 ssr_refraction;

    // Handle cases, where there is no refraction found
    if (refraction_coords.x > 1.0f || refraction_coords.x < 0.0f || refraction_coords.y > 1.0f || refraction_coords.y < 0.0f || refraction_coords.w == 0.0f)
    {
        ssr_refraction = float4(0.0f.xxxx);
    }
    else
    {
        ssr_refraction = rendered_scene.Sample(clamp_border_sampler, refraction_coords.xy);
    }

    if (ssr_refraction.w == 0.0f)
    {
        ssr_refraction = rendered_scene.Sample(clamp_border_sampler, UV);
    }

    // REFLECTION
    float3 reflected_vector = normalize(reflect(normalize(view_pos), view_reflection_normal));
    float4 reflection_coords = ray_cast(reflected_vector, view_pos, true);
    float4 ssr_reflection = rendered_scene.Sample(clamp_border_sampler, reflection_coords.xy);

    // Handle cases, where there is no reflection found
    if (reflection_coords.x > 1.0f || reflection_coords.x < 0.0f || reflection_coords.y > 1.0f || reflection_coords.y < 0.0f || reflection_coords.w == 0.0f)
    {
        ssr_reflection = float4(0.0f.xxx, 1.0f);
    }

    // Make reflection darker
    ssr_reflection = gamma_correction(ssr_reflection.xyz, 0.6);

    // Tint the water based on the height
    float3 pixel_color = float3(0.0f, 0.0f, 0.0f);
    float height = ((input.world_pos.y + combined_amplitude) / 2.0f) * (1.0f / combined_amplitude);
    pixel_color = top_color * height + bottom_color * (1.0f - height);

    float3 view_dir = normalize(camera_pos.xyz - input.world_pos.xyz);
    float3 result = calculate_directional_light(directional_light, combined_normal, view_dir, pixel_color, input.world_pos, true);

    // Volumetric lighting variables
    float3 scatter = float3(0.0f.xxx);
    float fog_value = fog_tex.Sample(wrap_sampler, UV + time_ps / 100.0f).r;

    for (int i = 0; i < number_of_point_lights; i++)
    {
        scatter += calculate_scatter(point_lights[i], float4(input.world_pos, 1.0f)) * fog_value;
        result += calculate_point_light(point_lights[i], combined_normal, input.world_pos.rgb, view_dir, pixel_color, i, RENDER_POINT_SHADOW_MAPS);
    }

    for (int j = 0; j < number_of_spot_lights; j++)
    {
        result += calculate_spot_light(spot_lights[j], combined_normal, input.world_pos, view_dir, pixel_color, j, true);
        scatter += calculate_scatter(spot_lights[j], float4(input.world_pos, 1.0f), j) * fog_value;
    }

    result = ((ssr_refraction.xyz) * 0.3f + result + ssr_reflection.xyz) / 3.0f;
    result += scatter;

    falloff_value = clamp(falloff_value, 0.0f, 1.0f);
    
    float4 final;
    final.xyz = falloff_value * float3(0.1f, 0.1f, 0.6f) + (1.0f - falloff_value) * result;
    final.xyz = gamma_correction(exposure_tonemapping(final.xyz));
    if (halo_value.a > 0.0f)
    {
        final.xyz += abs(halo_value.xyz);
    }
    final.a = 1.0f;
    return final;
}
