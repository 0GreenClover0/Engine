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
    float3 kernel_samples[64];
}

Texture2D pos_tex : register(t10);
Texture2D normal_tex : register(t11);
Texture2D diffuse_tex : register(t12);
Texture2D noise_tex : register(t13);

SamplerState gbuffer_sampler : register(s0);
SamplerState noise_sampler : register(s1);

const float2 noise_scale = float2(800.0f / 4.0f, 600.0f / 4.0f);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float3 world_pos = pos_tex.Sample(gbuffer_sampler, input.UV);
    float3 normal = normal_tex.Sample(gbuffer_sampler, input.UV);
    float3 random_vec = noise_tex.Sample(noise_sampler, input.UV * noise_scale);

    float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    float radius = 300.0f; // Tweakable
    float bias = 0.25f; // Tweakable
    for (int i = 0; i < 64; ++i)
    {
        float3 sample_pos = mul(TBN, kernel_samples[i]);
        sample_pos = world_pos + sample_pos * radius;

        float4 offset = float4(sample_pos, 1.0f);
        offset = mul(projection, offset); // From view to clip-space
        offset.xyz /= offset.w; // Perspective divide
        offset.xyz  = offset.xyz * 0.5f + 0.5f; // Transform to range 0.0 - 1.0  

        float sample_depth = pos_tex.Sample(gbuffer_sampler, offset.xy).z;
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(world_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0f : 0.0f) * rangeCheck;
    }

    occlusion = 1.0f - (occlusion / 64.0f);
    occlusion = pow(occlusion, 1.5f);
    return float4(occlusion, occlusion, occlusion, occlusion);
}
