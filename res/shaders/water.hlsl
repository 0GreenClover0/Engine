#define PI 3.141592f
#define gravity 9.81f

#include "structs.hlsl"

cbuffer light_buffer : register(b0)
{
    DirectionalLight directonal_light;
    PointLight point_lights[4];
    SpotLight spot_lights[4];
    float3 camera_pos;
    int number_of_point_lights;
    int number_of_spot_lights;
};

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
    float4x4 light_projection_view_model;
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
    float4 light_space_pos : TEXCOORD1;
    float3 raw_pos : TEXCOORD2;
};

struct PositionAndNormal
{
    float3 position;
    float3 normal;
};

Texture2D obj_texture : register(t0);
Texture2D directional_shadow_map : register(t1);

// This is not ideal, however my options were:
// - using a TextureCubeArray, which would require me to pass the texture cubes as one resource (which is doable, but it would require another refactor)
// - using an array of TextureCubes, but in this case you can't use them like a normal array anyway (e.g. point_light_shadow_maps[0]), so what's the point.
// Because we have a limited number of point-lights, I just kind of brute-forced it.
TextureCube point_light_shadow_map_slot0 : register(t2);
TextureCube point_light_shadow_map_slot1 : register(t3);
TextureCube point_light_shadow_map_slot2 : register(t4);
TextureCube point_light_shadow_map_slot3 : register(t5);

Texture2D spot_light_shadow_map_slot0 : register(t6);
Texture2D spot_light_shadow_map_slot1 : register(t7);
Texture2D spot_light_shadow_map_slot2 : register(t8);
Texture2D spot_light_shadow_map_slot3 : register(t9);

SamplerState obj_sampler_state : register(s0);
SamplerState shadow_map_sampler : register(s1);

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
    output.light_space_pos = mul(light_projection_view_model, float4(input.pos, 1.0f));
    output.raw_pos = input.pos;
    return output;
}

float3 calculate_directional_light(DirectionalLight light, float3 normal, float3 view_dir, float3 diffuse_texture, float4 light_space_pos)
{
    float3 light_direction = normalize(-light.direction);

    // Diffuse
    float3 diff = max(dot(normal, light_direction), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_direction,normal); // Phong
    float3 halfway_dir = normalize(light_direction + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32); // TODO: Take shininess from the material

    float3 ambient = light.ambient * diffuse_texture; // We should be sampling diffuse map
    float3 diffuse = light.diffuse * diff * diffuse_texture; // We should be sampling diffuse map
    float3 specular = light.specular * spec * diffuse_texture; // We should be sampling specular map

    return ambient + diffuse + specular;
}

float3 calculate_point_light(PointLight light, float3 normal, float3 world_pos, float3 view_dir, float3 diffuse_texture, int index)
{
    float3 light_dir = normalize(light.position - world_pos);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_dir,normal); // Phong
    float3 halfway_dir = normalize(light_dir + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32);

    // Attenuation
    float distance = length(light.position.xyz - world_pos);
    float attenuation = 1.0f / (light.constant + light.linear_ * distance + light.quadratic * (distance * distance));

    float3 ambient = light.ambient * diffuse_texture;
    float3 diffuse = light.diffuse * diff * diffuse_texture;
    float3 specular = light.specular * spec * diffuse_texture;
    return attenuation * (ambient + diffuse + specular);
}

float3 calculate_spot_light(SpotLight light, float3 normal, float3 world_pos, float3 view_dir, float3 diffuse_texture, int index, float3 raw_pos)
{
    float3 light_dir = normalize(light.position - world_pos.xyz);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_dir,normal); // Phong
    float3 halfway_dir = normalize(light_dir + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32);

    // Attenuation
    float distance = length(light.position - world_pos);
    float attenuation = 1.0f / (light.constant + light.linear_ * distance + light.quadratic * distance * distance);

    // Spotlight intensity
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0f , 1.0f);

    float3 ambient = light.ambient * diffuse_texture;
    float3 diffuse = light.diffuse * diff * diffuse_texture;
    float3 specular = light.specular * spec * diffuse_texture;

    return attenuation * intensity * (ambient + diffuse + specular);
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float3 norm = normalize(input.normal);
    
    float3 view_dir = normalize(camera_pos.xyz - input.world_pos.xyz);
    float3 diffuse_texture = obj_texture.Sample(obj_sampler_state, input.UV).rgb;

    float3 result = calculate_directional_light(directonal_light, norm, view_dir, diffuse_texture, input.light_space_pos);

    for (int i = 0; i < number_of_point_lights; i++)
    {
        result += calculate_point_light(point_lights[i], norm, input.world_pos.rgb, view_dir, diffuse_texture, i);
    }

    for (int j = 0; j < number_of_spot_lights; j++)
    {
        result += calculate_spot_light(spot_lights[j], norm, input.world_pos, view_dir, diffuse_texture, j, input.raw_pos);
    }

    return float4(result, .8f);
}
