struct VS_Input
{
	float2 position: POSITION;
	float2 uv: UV;
};

struct VS_Output
{
	float4 position: SV_POSITION;
	float2 uv: UV;
};

VS_Output vs_main(VS_Input input)
{
	VS_Output output;
	output.position = float4(input.position, 0.0f, 1.0f);
	output.uv = input.uv;
	return output;
};

Texture2D my_texture;
SamplerState my_sampler;

float4 ps_main(VS_Output input): SV_TARGET
{	
	return my_texture.Sample(my_sampler, input.uv);
};