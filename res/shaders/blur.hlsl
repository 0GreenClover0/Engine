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

Texture2D texture_to_blur : register(t14);
SamplerState default_sampler_state : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float texel_width;
    float texel_height;
    texture_to_blur.GetDimensions(texel_width, texel_height);
    texel_width = 1.0f / texel_width;
    texel_height = 1.0f / texel_height;

    float result = 0.0f;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            float2 offset = float2(x * texel_width, y * texel_height);
            result += texture_to_blur.Sample(default_sampler_state, input.UV + offset).r;
        }
    }

    result = result / 16.0f;

    return float4(result, result, result, result);
}

