#include "structs.hlsl"
#include "common_functions.hlsl"

cbuffer light_buffer : register(b0)
{
    DirectionalLight directional_light;
    PointLight point_lights[20];
    SpotLight spot_lights[20];
    float3 camera_pos;
    int number_of_point_lights;
    int number_of_spot_lights;
};

SamplerState shadow_map_sampler : register(s1);

Texture2D directional_shadow_map : register(t1);
Texture2D spot_light_shadow_maps[20] : register(t20);
TextureCube point_light_shadow_maps[20]: register(t40);

float point_shadow_calculation(PointLight light, float3 world_pos, int index)
{
    float3 pixel_to_light = world_pos - light.position;
    float closest_depth = 0.0f;
    closest_depth = point_light_shadow_maps[index].SampleLevel(shadow_map_sampler, pixel_to_light, 0).r;
    closest_depth *= light.far_plane;
    float current_depth = length(pixel_to_light);
    float shadow = current_depth - 0.05f > closest_depth ? 1.0f : 0.0f;
    return shadow;
}

float spot_shadow_calculation(SpotLight light, float3 world_pos, int index, float3 normal)
{
    float4 light_space_pos = mul(light.projection_view, float4(world_pos, 1.0f));
    light_space_pos.xyz /= light_space_pos.w;
    light_space_pos.x = light_space_pos.x / 2.0f + 0.5f;
    light_space_pos.y = -light_space_pos.y / 2.0f + 0.5f;
    float shadow = 0.0f;
    float bias = max(0.005f * (1.0f - dot(normal, light.direction)), 0.005f);
    float depth = light_space_pos.z;

    // Reference for PCF implementation:
    // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    float2 map_size;
    spot_light_shadow_maps[index].GetDimensions(map_size.x, map_size.y);
    float2 texel_size = 1.0 / map_size;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcf_depth = spot_light_shadow_maps[index].SampleLevel(shadow_map_sampler, light_space_pos.xy + float2(x, y) * texel_size, 0).r;
            shadow += depth - bias > pcf_depth ? 1.0f : 0.0f;
        }
    }

    shadow /= 9.0f;

    return shadow;
}

float directional_shadow_calculation(DirectionalLight light, float3 world_pos, float3 normal)
{
    float4 light_space_pos = mul(directional_light.projection_view, float4(world_pos, 1.0f));
    light_space_pos.xyz /= light_space_pos.w;

    float depth = light_space_pos.z;
    if (depth > 1.0f)
    {
        return 0.0f;
    }

    light_space_pos.x = light_space_pos.x / 2.0f + 0.5f;
    light_space_pos.y = -light_space_pos.y / 2.0f + 0.5f;
    float shadow = 0.0f;
    float bias = max(0.005f * (1.0f - dot(normal, light.direction)), 0.005f);

    // Reference for PCF implementation:
    // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    float2 map_size;
    directional_shadow_map.GetDimensions(map_size.x, map_size.y);
    float2 texel_size = 1.0 / map_size;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcf_depth = directional_shadow_map.SampleLevel(shadow_map_sampler, light_space_pos.xy + float2(x, y) * texel_size, 0).r;
            shadow += depth - bias > pcf_depth ? 1.0f : 0.0f;
        }
    }

    shadow /= 9.0f;

    return shadow;
}

float3 calculate_directional_light(DirectionalLight light, float3 normal, float3 view_dir, float3 diffuse_texture, float3 world_pos, bool calculate_shadows, float ambient_occlusion = 1.0f)
{
    float3 light_direction = normalize(-light.direction);
    // Diffuse
    float3 diff = max(dot(normal, light_direction), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_direction,normal); // Phong
    float3 halfway_dir = normalize(light_direction + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32); // TODO: Take shininess from the material

    float3 ambient = light.ambient * diffuse_texture * ambient_occlusion; // We should be sampling diffuse map
    float3 diffuse = light.diffuse * diff * diffuse_texture; // We should be sampling diffuse map
    float3 specular = light.specular * spec * diffuse_texture; // We should be sampling specular map

    float shadow = 0.0f;
    if (calculate_shadows)
    {
        shadow = directional_shadow_calculation(light, world_pos, normal);
    }

    return ambient + (1.0f - shadow) * (diffuse + specular);
}

float3 calculate_point_light(PointLight light, float3 normal, float3 world_pos, float3 view_dir, float3 diffuse_texture, int index, bool calculate_shadows)
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

    float shadow = 0.0f;
    if (calculate_shadows)
    {
        shadow = point_shadow_calculation(light, world_pos, index);
    }

    return attenuation * (ambient + (1.0f - shadow) * (diffuse + specular));
}

float3 calculate_spot_light(SpotLight light, float3 normal, float3 world_pos, float3 view_dir, float3 diffuse_texture, int index, bool calculate_shadows)
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

    float shadow = 0.0f;
    if (calculate_shadows)
    {
        shadow = spot_shadow_calculation(light, world_pos, index, normal);
    }

    return attenuation * intensity * (ambient + (1.0f - shadow) * (diffuse + specular));
}
