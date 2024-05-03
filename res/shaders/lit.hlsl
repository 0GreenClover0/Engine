struct PointLight
{
    float3 position;
    float3 ambient;
    float3 diffuse;
    float3 specular;
    
    float constant;
    float linear_;
    float quadratic;

    float far_plane;
    float near_plane;
};

struct DirectionalLight
{
    float3 direction;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

struct SpotLight
{
    float3 position;
    float3 direction;
    float cut_off;
    float outer_cut_off;

    float constant;
    float linear_;
    float quadratic; 

    float3 ambient;
    float3 diffuse;
    float3 specular;
};

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
};

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
};

Texture2D obj_texture : register(t0);
Texture2D directional_shadow_map : register(t1);

// This is not ideal, however my options were:
// - using a TextureCubeArray, which would require me to pass the texture cubes as one resource (which is doable, but it would require another refactor)
// - using an array of TextureCubes, but in this case you can't use them like a normal array anyway (e.g. shadow_cube_maps[0]), so what's the point.
// Because we have a limited number of point-lights, I just kind of brute-forced it.
TextureCube shadow_cube_map_slot0 : register(t2);
TextureCube shadow_cube_map_slot1 : register(t3);
TextureCube shadow_cube_map_slot2 : register(t4);
TextureCube shadow_cube_map_slot3 : register(t5);

SamplerState obj_sampler_state : register(s0);
SamplerState shadow_map_sampler : register(s1);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.normal = mul(input.normal, (float3x3)model);
    output.pixel_pos = mul(projection_view_model, float4(input.pos, 1.0f));
    output.light_space_pos = mul(light_projection_view_model, float4(input.pos, 1.0f));
    return output;
}

float point_shadow_calculation(PointLight light, float3 world_pos, int index)
{
    float3 pixel_to_light = world_pos - light.position;
    float closest_depth;
    if (index == 0)
    {
        closest_depth = shadow_cube_map_slot0.Sample(shadow_map_sampler, pixel_to_light).r;
    }
    else if (index == 1)
    {
        closest_depth = shadow_cube_map_slot1.Sample(shadow_map_sampler, pixel_to_light).r;
    }
    else if (index == 2)
    {
        closest_depth = shadow_cube_map_slot2.Sample(shadow_map_sampler, pixel_to_light).r;
    }
    else if (index == 3)
    {
        closest_depth = shadow_cube_map_slot3.Sample(shadow_map_sampler, pixel_to_light).r;
    }

    closest_depth *= light.far_plane;
    float current_depth = length(pixel_to_light);
    float shadow = current_depth - 0.05f > closest_depth ? 1.0f : 0.0f;
    return shadow;
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

    light_space_pos.xyz /= light_space_pos.w;
    float depth = light_space_pos.z;

    if (depth > 1.0f)
    {
        return ambient + diffuse + specular;
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
            float pcf_depth = directional_shadow_map.Sample(shadow_map_sampler, light_space_pos.xy + float2(x, y) * texel_size).r;
            shadow += depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return ambient + (1.0f - shadow) * (diffuse + specular);
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
    float shadow = point_shadow_calculation(light, world_pos, index);
    return attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));
}

float3 calculate_spot_light(SpotLight light, float3 normal, float3 world_pos, float3 view_dir, float3 diffuse_texture)
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

    return attenuation * intensity * (ambient + specular + diffuse);
}

float4 gamma_correction(float3 color)
{
    // If screen is too bright it might be because DirectX can do gamma correction by itself
    // However, it works only in fullscreen
    // so keep that in mind just in case
    float gamma_factor = 1.0f / 2.2f;
    float3 sRGB_color = float3(pow(color.x, gamma_factor), pow(color.y, gamma_factor), pow(color.z, gamma_factor));
    return float4(sRGB_color.xyz, 1.0f);
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
        result += calculate_spot_light(spot_lights[j], norm, input.world_pos, view_dir, diffuse_texture);
    }
    
    return gamma_correction(result);
}
