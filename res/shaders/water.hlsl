#define PI 3.141592f
#define gravity 9.86f

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

Texture2D obj_texture : register(t0);
Texture2D directional_shadow_map : register(t1);
Texture2D water_normal_map : register(t2);
/* This is not ideal, however my options were:
- using a TextureCubeArray, which would require me to pass the texture cubes as one resource (which is doable, but it would require another refactor)
- using an array of TextureCubes, but in this case you can't use them like a normal array anyway (e.g. point_light_shadow_maps[0]), so what's the point.
Because we have a limited number of point-lights, I just kind of brute-forced it. If we decide to have more than 4 point lights at once we might as well call NASA for a better PC.
*/
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
float3 calc_gerstner_wave_position(float x, float y)
{
    float3 pos;
    pos.x = x;
    pos.z = y;
    pos.y = 0.0f;
    for(int i = 0; i < no_of_waves; i++)
    {
        float speed = waves[i].speed;
        // main variables
        float wave_length = waves[i].wave_length;
        float amplitude = waves[i].amplitude;
        float2 direction = waves[i].direction;

        float frequency = sqrt(gravity * 2.0f * PI / wave_length);
        float steepness = waves[i].steepness / (frequency * amplitude * no_of_waves);
        //float frequency =  2.0f / wave_length;
        float phi = speed * 2.0f / wave_length;
        pos.x += steepness * amplitude * direction.x * cos(dot((frequency * direction),float2(x,y)) + phi * time);
        pos.z += steepness * amplitude * direction.y * cos(dot((frequency * direction),float2(x,y)) + phi * time);
        pos.y += amplitude * sin(dot((frequency * direction), float2(x,y)) + phi * time);
    }
    return pos;
}
float3 calc_gerstner_wave_normals(float3 position)
{
    float3 normal = float3(0.0f, 1.0f, 0.0f);
    // fix this shit
    for(int i = 0; i < no_of_waves; i++)
    {
        float speed = waves[i].speed;
        // main variables
        float wave_length = waves[i].wave_length;
        float amplitude = waves[i].amplitude;
        float2 direction = waves[i].direction;

        float frequency = sqrt(gravity * 2.0f * PI / wave_length);
        float steepness = waves[i].steepness / (frequency * amplitude * no_of_waves);
        //float frequency =  2.0f / wave_length;
        float phi = speed * 2.0f / wave_length;
        // variable naming from:
        //https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
        float WA = frequency * amplitude;
        float S_func = sin(frequency * dot(direction, position.xz) + phi * time);
        float C_func = cos(frequency * dot(direction, position.xz) + phi * time);
        
        normal.x -= direction.x * WA * C_func;
        normal.z -= direction.y * WA * C_func;
        normal.y -= steepness * WA * S_func;
    }
    return normal;
}



VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    input.pos = calc_gerstner_wave_position(input.pos.x, input.pos.z);
    input.normal = calc_gerstner_wave_normals(input.pos);
    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.normal = mul(input.normal, (float3x3)model);
    output.pixel_pos = mul(projection_view_model, float4(input.pos, 1.0f));
    output.light_space_pos = mul(light_projection_view_model, float4(input.pos, 1.0f));
    output.raw_pos = input.pos;
    return output;
}

// PIXEL SHADER FUNCTIONS
float point_shadow_calculation(PointLight light, float3 world_pos, int index)
{
    float3 pixel_to_light = world_pos - light.position;
    float closest_depth;
    if (index == 0) closest_depth = point_light_shadow_map_slot0.Sample(shadow_map_sampler, pixel_to_light).r;
    else if (index == 1) closest_depth = point_light_shadow_map_slot1.Sample(shadow_map_sampler, pixel_to_light).r;
    else if (index == 2) closest_depth = point_light_shadow_map_slot2.Sample(shadow_map_sampler, pixel_to_light).r;
    else if (index == 3) closest_depth = point_light_shadow_map_slot3.Sample(shadow_map_sampler, pixel_to_light).r;
    closest_depth *= light.far_plane;
    float current_depth = length(pixel_to_light);
    float shadow = current_depth - 0.05 > closest_depth ? 1.0 : 0.0;
    return shadow;
}

float spot_shadow_calculation(SpotLight light, float3 raw_pos, int index, float3 normal)
{
    float4 light_space_pos = mul(light.projection_view_model, float4(raw_pos, 1.0f));

    light_space_pos.xyz /= light_space_pos.w;
    light_space_pos.x = light_space_pos.x / 2.0f + 0.5f;
    light_space_pos.y = -light_space_pos.y / 2.0f + 0.5f;
    float shadow = 0.0f;
    float bias = max(0.005f * (1.0f - dot(normal, light.direction)), 0.005f);
    float depth = light_space_pos.z;

    // Reference for PCF implementation:
    // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    Texture2D current_texture;
    if (index == 0) current_texture = spot_light_shadow_map_slot0;
    else if (index == 1) current_texture = spot_light_shadow_map_slot1;
    else if (index == 2) current_texture = spot_light_shadow_map_slot2;
    else if (index == 3) current_texture = spot_light_shadow_map_slot3;
    
    float2 map_size;
    spot_light_shadow_map_slot0.GetDimensions(map_size.x, map_size.y);
    float2 texel_size = 1.0 / map_size;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcf_depth = spot_light_shadow_map_slot0.Sample(shadow_map_sampler, light_space_pos.xy + float2(x, y) * texel_size).r;
            shadow += depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
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
    float shadow = spot_shadow_calculation(light, raw_pos, index, normal);

    return attenuation * intensity * (ambient + (1.0 - shadow) * (diffuse + specular));
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
