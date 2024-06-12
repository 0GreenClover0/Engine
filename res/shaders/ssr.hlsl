#include "lighting_calculations.hlsl"

Texture2D rendered_scene : register(t17);
Texture2D position_buffer : register(t10);

SamplerState wrap_sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
};

#define refraction_ray_step 0.10f
#define refraction_max_steps 70
#define refraction_thickness 0.2f
#define num_binary_search_steps 10

float3 fresnel_schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 binary_search(float3 dir, float3 hit_coords, float dDepth)
{
    float depth;
    float4 projected_coord;

    [loop]
    for (int i = 0; i < num_binary_search_steps; ++i)
    {
        projected_coord = mul(projection, float4(hit_coords, 1.0f));
        projected_coord.xy /= projected_coord.w;
        projected_coord.xy = projected_coord.xy * 0.5f + 0.5f;
        projected_coord.y *= -1.0f;
        projected_coord.y += 1.0f;
        float3 view_pos = position_buffer.Sample(wrap_sampler, projected_coord.xy).xyz;
        view_pos = mul(view, float4(view_pos, 1.0f));
        depth = view_pos.z;

        dDepth = hit_coords.z - depth;
        dir *= 0.5f;

        if (dDepth > 0.0f)
        {
            hit_coords += dir;
        }
        else
        {
            hit_coords -= dir;
        }
    }

    projected_coord = mul(projection, float4(hit_coords, 1.0f));
    projected_coord.xy /= projected_coord.w;
    projected_coord.xy = projected_coord.xy * 0.5f + 0.5f; 
    projected_coord.y *= -1.0f;
    projected_coord.y += 1.0f;

    return float3(projected_coord.xy, depth);
}

float4 ray_cast(float3 dir, float3 hit_coord)
{
    dir *= refraction_ray_step;

    float depth;
    int steps = 0;
    float4 projected_coord;
    float dDepth; // Not sure what this is supposed to mean

    [loop]
    for (int i = 0; i < refraction_max_steps; ++i)
    {
        hit_coord += dir;

        projected_coord = mul(projection, float4(hit_coord, 1.0f));
        // Perspective divide
        projected_coord.xy /= projected_coord.w;
        // Transform to NDC
        projected_coord.xy = projected_coord.xy * 0.5f + 0.5f;
        projected_coord.y *= -1.0f;
        projected_coord.y += 1.0f;

        float3 view_pos = position_buffer.Sample(wrap_sampler, projected_coord.xy).xyz;
        view_pos = mul(view, float4(view_pos, 1.0f));
        depth = view_pos.z;

        if (depth > 1000.0f)
        {
            continue;
        }

        // How big is the difference between the ray depth and fetched depth?
        dDepth = hit_coord.z - depth;

        // If over a threshold the object will reproject into infinity
        if (dir.z - dDepth < refraction_thickness)
        {
            if (dDepth <= 0.0f)
            {
                float4 result;
                result = float4(binary_search(dir, hit_coord, dDepth), 1.0f);
                return result;
            }
        }

        steps++;
    }

    // If we ever get here, it means we couldn't get anywhere close a surface
    return float4(projected_coord.xy, depth, 0.0f);
}