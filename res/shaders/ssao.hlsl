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

cbuffer ssao_buffer : register(b1)
{
    float4x4 projection;
    float4x4 view;
    float3 kernel_samples[64];
}

Texture2D pos_tex : register(t10);
Texture2D normal_tex : register(t11);
Texture2D diffuse_tex : register(t12);
Texture2D noise_tex : register(t13);

SamplerState gbuffer_sampler : register(s0);
SamplerState noise_sampler : register(s1);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

// Based on https://learnopengl.com/Advanced-Lighting/SSAO
//          https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/9.ssao/9.ssao.fs
float4 ps_main(VS_Output input) : SV_Target
{
    float3 world_pos = pos_tex.Sample(gbuffer_sampler, input.UV);
    
    float2 noise_scale;
    pos_tex.GetDimensions(noise_scale.x, noise_scale.y);
    noise_scale /= 4.0f;

    float4 view_pos = mul(view, float4(world_pos, 1.0f));
    view_pos.xyz /= view_pos.w;
    float3 normal = normalize(normal_tex.Sample(gbuffer_sampler, input.UV).xyz);
    normal = normalize(mul((float3x3)view, normal));
    float3 random_vec = normalize(noise_tex.Sample(noise_sampler, input.UV * noise_scale).xyz);
    float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    float radius = 1.5f;
    float bias = 0.001f;
    for (int i = 0; i < 16; ++i)
    {
        float3 sample_pos = mul(TBN, kernel_samples[i]);
        sample_pos = view_pos.xyz + sample_pos * radius;

        float4 offset = float4(sample_pos, 1.0f);
        offset = mul(projection, offset); // From view to clip-space
        offset.xyz /= offset.w; // Perspective division
        offset.xyz = offset.xyz * 0.5f + 0.5f; // From clip-space to texture coordinates
        offset.xy = float2(offset.x, 1.0f - offset.y); // Flip y-axis
        float4 sample_depth = mul(view, float4(pos_tex.Sample(gbuffer_sampler, offset.xy).xyz, 1.0f));
        float range_check = smoothstep(0.0f, 1.0f, radius / abs(view_pos.z - sample_depth.z));
        if (sample_depth.z >= sample_pos.z + bias)
        {
            occlusion += range_check;
        }
    }

    occlusion = 1.0f - (occlusion / 16.0f);
    occlusion = pow(occlusion, 3.0f);
    return float4(occlusion, occlusion, occlusion, occlusion);
}
