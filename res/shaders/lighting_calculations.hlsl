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

cbuffer ps_time_buffer : register(b3)
{
    float time_ps;
    bool is_fog_rendered;
}

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

float spot_shadow_calculation(SpotLight light, float3 world_pos, int index, float3 normal, bool smooth)
{
    float4 light_space_pos = mul(light.projection_view, float4(world_pos, 1.0f));
    light_space_pos.xyz /= light_space_pos.w;
    light_space_pos.x = light_space_pos.x / 2.0f + 0.5f;
    light_space_pos.y = -light_space_pos.y / 2.0f + 0.5f;
    float shadow = 0.0f;
    float depth = light_space_pos.z;

    if (smooth)
    {
        // Reference for PCF implementation:
        // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
        float2 map_size;
        spot_light_shadow_maps[index].GetDimensions(map_size.x, map_size.y);
        float2 texel_size = 1.0 / map_size;
        float bias = max(0.005f * (1.0f - dot(normal, light.direction)), 0.005f);
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                float pcf_depth = spot_light_shadow_maps[index].SampleLevel(shadow_map_sampler, light_space_pos.xy + float2(x, y) * texel_size, 0).r;
                shadow += depth - bias > pcf_depth ? 1.0f : 0.0f;
            }
        }
        shadow /= 9.0f;
    }
    else
    {
        float shadow_map_depth = spot_light_shadow_maps[index].SampleLevel(shadow_map_sampler, light_space_pos.xy, 0).r;
        float bias = 0.0f;
        if (shadow_map_depth < depth + bias)
        {
            shadow = 1.0f;
        }
    }
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
        shadow = spot_shadow_calculation(light, world_pos, index, normal, true);
    }

    return attenuation * intensity * (ambient + (1.0f - shadow) * (diffuse + specular));
}

float2 intersect_light_cone(SpotLight light, float3 ray_origin, float3 ray_direction, out float shadow, float light_index)
{
    float4 local_origin = mul(light.inv_model, float4(ray_origin, 1.0f));
    float4 local_direction = mul(light.inv_model, float4(ray_direction, 0.0f));

    // We need to use acos because outer_cut_off is a cosine value of the outer cut off angle
    float tan_theta = tan(acos(light.outer_cut_off));
    tan_theta *= tan_theta;
    float a = local_direction.x * local_direction.x + local_direction.z * local_direction.z - local_direction.y * local_direction.y * tan_theta;
    float b = 2.0f * (local_origin.x * local_direction.x + local_origin.z * local_direction.z - local_origin.y * local_direction.y * tan_theta);
    float c = local_origin.x * local_origin.x + local_origin.z * local_origin.z - local_origin.y * local_origin.y * tan_theta;

    float min_t = 0.0f;
    float max_t = 0.0f;
    solve_quadratic(a, b, c, min_t, max_t);

    float y1 = local_origin.y + local_direction.y * min_t;
    float y2 = local_origin.y + local_direction.y * max_t;

    float bias = 0.01f;
    float3 local_point_min = local_origin.xyz + (min_t + bias) * local_direction.xyz;
    float3 local_point_max = local_origin.xyz + (max_t - bias) * local_direction.xyz;

    float4 world_point_min_h = mul(light.model, float4(local_point_min, 1.0f));
    world_point_min_h.xyz /= world_point_min_h.w;
    float4 world_point_max_h = mul(light.model, float4(local_point_max, 1.0f));
    world_point_max_h.xyz /= world_point_max_h.w;

    // Shadow is present, when the closest intersection point is in the shadow
    shadow = spot_shadow_calculation(light, world_point_min_h.xyz, light_index, float3(1.0f, 1.0f, 1.0f), false);

    if (y1 > 0.0f && y2 > 0.0f)
    {
        // Both intersections are in the reflected cone so return degenerate value
        min_t = 0.0f;
        max_t = -1.0f;
    }
    else if (y1 > 0.0f && y2 < 0.0f)
    {
        // Closest t on the wrong side, furthest on the right side => ray enters volume but doesn't leave it (so set max_t arbitrarily large)
        min_t = max_t;
        max_t = 10000.0f;
    }
    else if (y1 < 0.0f && y2 > 0.0f)
    {
        // closest t on the right side, largest on the wrong side => ray starts in volume and exits once
        max_t = min_t;
        min_t = 0.0f;
    }

    return float2(min_t, max_t);
}

float in_scatter(float3 start, float3 dir, float3 lightPos, float d)
{
    // Calculate quadratic coefficients a, b, c
    float3 q = start - lightPos;

    float b = dot(dir, q);
    float c = dot(q, q);

    // Evaluate integral
    float denominator = c - b * b;

    // Avoid division by zero or very small values
    if (denominator < 1e-6)
    {
        // Return arbitarly large value, because when the denominator is very small
        // it means the pixel is very close to the light source
        return 100000.0f;
    }

    float s = 1.0f / sqrt(denominator);
    float l = s * (atan((d + b) * s) - atan(b * s));

    return l;
}

float3 calculate_scatter(SpotLight light, float4 world_position, int light_index)
{
    float3 surface_to_camera_direction = world_position.xyz - camera_pos;
    float ray_length = length(surface_to_camera_direction);
    surface_to_camera_direction /= ray_length;

    float aperture = light.outer_cut_off;
    float min_t = 0.0f;
    float max_t = 0.0f;

    float shadow = 0.0f;
    float2 res = intersect_light_cone(light, camera_pos, surface_to_camera_direction, shadow, light_index);

    if (shadow > 0.0f)
    {
        return 0.0f;
    }

    min_t = res.x;
    max_t = res.y;
    max_t = clamp(max_t, 0.0f, ray_length);
    min_t = max(0.0f, min_t);

    float t = max(0.0f, max_t - min_t);
    float scattering_coefficient = 100.0f;

    // According to Rayliegh scattering blue light at the lower end of the spectrum is scattered considerably more than red light.
    // It's simple to account for this wavelength dependence by making the scattering coefficient a constant vector
    // weighted towards the blue component.  I found this helps add to the realism of the effect.
    // src: https://blog.mmacklin.com/2010/05/29/in-scattering-demo/
    float3 scattering_constants = float3(0.2f, 0.4f, 0.8f);
    float3 scatter = light.diffuse * scattering_constants * in_scatter(camera_pos + surface_to_camera_direction * min_t, surface_to_camera_direction, light.position, t) * scattering_coefficient;
    return scatter;
}

float3 calculate_scatter(PointLight light, float4 world_position)
{
    float3 surface_to_camera_direction = world_position.xyz - camera_pos;
    float ray_length = length(surface_to_camera_direction);
    surface_to_camera_direction /= ray_length;

    float light_to_pixel = length(light.position - world_position.xyz);
    float3 scattering_constants = float3(0.2f, 0.4f, 0.8f);
    // This is arbitrary, however connected with light's attenuation
    float scattering_coefficient = 0.3f / (light.constant + light.linear_ * light_to_pixel + light.quadratic * light_to_pixel * light_to_pixel);
    float3 scatter = light.diffuse * scattering_constants * in_scatter(camera_pos, surface_to_camera_direction, light.position, ray_length) * scattering_coefficient;
    return scatter;
}
