struct PointLight
{
    float3 position;
    float3 ambient;
    float3 diffuse;
    float3 specular;
    
    float constant;
    float linear_;
    float quadratic;
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
};

Texture2D obj_texture;
SamplerState obj_sampler_state;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.normal = normalize(mul(input.normal, (float3x3)model));
    output.pixel_pos = mul(projection_view_model, float4(input.pos.xyz, 1.0f));

    return output;
}

float3 calculate_directional_light(DirectionalLight light, float3 normal, float3 view_dir, float3 diffuse_texture)
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

float3 calculate_point_light(PointLight light, float3 normal, float3 pixel_pos, float3 view_dir, float3 diffuse_texture)
{
    float3 light_dir = normalize(light.position - pixel_pos);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_dir,normal); // Phong
    float3 halfway_dir = normalize(light_dir + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32);

    // Attenuation
    float distance = length(light.position.xyz - pixel_pos);
    float attenuation = 1.0f / (light.constant + light.linear_ * distance + light.quadratic * (distance * distance));

    float3 ambient = light.ambient * diffuse_texture;
    float3 diffuse = light.diffuse * diff * diffuse_texture;
    float3 specular = light.specular * spec * diffuse_texture;

    return attenuation * (ambient + diffuse + specular);
}

float3 calculate_spot_light(SpotLight light, float3 normal, float3 pixel_pos, float3 view_dir, float3 diffuse_texture)
{
    float3 light_dir = normalize(light.position - pixel_pos.xyz);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0f);

    // Specular
    //float3 reflect_dir = reflect(-light_dir,normal); // Phong
    float3 halfway_dir = normalize(light_dir + view_dir); // Blinn-Phong
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0f), 32);

    // Attenuation
    float distance = length(light.position - pixel_pos);
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

float4 ps_main(VS_Output input) : SV_TARGET
{
    float3 norm = normalize(input.normal);
    float3 view_dir = normalize(camera_pos.xyz - input.world_pos.xyz);
    float3 diffuse_texture = obj_texture.Sample(obj_sampler_state, input.UV).rgb;

    float3 result = calculate_directional_light(directonal_light, norm, view_dir, diffuse_texture);

    for (int i = 0; i < number_of_point_lights; i++)
    {
        result += calculate_point_light(point_lights[i], norm, input.world_pos.rgb, view_dir, diffuse_texture);
    }

    for (int j = 0; j < number_of_spot_lights; j++)
    {
        result += calculate_spot_light(spot_lights[j], norm, input.world_pos, view_dir, diffuse_texture);
    }

    return float4(result, 1.0f);
}
