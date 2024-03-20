struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 position : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = float4(input.pos.xy, 0.0f, 1.0f);

    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
