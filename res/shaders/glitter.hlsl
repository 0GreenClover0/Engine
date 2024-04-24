cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 world;
};

cbuffer time_buffer : register(b1)
{
    double time;
};

struct VS_Input
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD0;
    float3 world_pos : TEXCOORD1;
};

 Texture2D ObjTexture;
 SamplerState ObjSamplerState;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(projection_view_model, float4(input.pos.xyz, 1.0f));
    output.normal = input.normal;
    output.world_pos = mul(float4(input.pos, 1.0), projection_view_model).xyz;
    output.UV = input.UV;
    return output;
}

float hash(float n)
{
    return frac(sin(n) * 43758.5453);
}

float noise(float3 x)
{
    float x1 = x * time;
    float3 p = floor(x);
    float3 f = frac(x);
    f = f * f * (3.0 - 2.0 * f);

    float n = (p.x + p.y * 57.0 + 113.0 * p.z);
    return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
                    lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
            lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
                    lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float n = noise(input.world_pos * 10.20);
    float glitter = (n > 0.98 ? 1.0 : 0.0);
    
    float4 base_color = ObjTexture.Sample(ObjSamplerState, input.UV);
    return base_color * (1.0 + glitter * 0.75);
}