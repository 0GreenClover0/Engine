const int noOfPointLights = 1;
const int noOfSpotLights = 1;

struct PointLight
{
    float4 pos;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 constant_lin_quad;
};


cbuffer mycBuffer : register(b0)
{
    float4x4 projection_view;
	float4x4 model;
};
cbuffer lightBuffer : register(b0)
{
    PointLight pointLights;
    float4 camera_pos;
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

Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    float4 temp = float4(input.pos, 1.0f);
    float4 temp2 = mul(model, temp);
    output.world_pos = temp2;
    output.UV = input.UV;
    float3 worldNormal = mul(input.normal, (float3x3)model);
    output.normal = normalize(worldNormal);
    //output.normal = input.normal;
    float4x4 pvm = mul(projection_view, model);
    output.pixel_pos = mul(pvm, float4(input.pos.xyz, 1.0f));

    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float3 result = float3(0.0f, 0.0f, 0.0f);
    float3 ambient = pointLights.ambient.xyz * ObjTexture.Sample(ObjSamplerState, input.UV).rgb;
    result += ambient;
    
	for(int i = 0; i < 1; i++)
    {
        //diffuse
        float3 norm = normalize(input.normal);
        float3 lightDir = normalize(pointLights.pos.xyz - input.world_pos.xyz);
        float diff = max(dot(norm, lightDir), 0.0f);
        float3 diffuse = pointLights.diffuse.xyz * diff * ObjTexture.Sample(ObjSamplerState, input.UV).rgb;
        //specular
        float3 viewDir = normalize(camera_pos.xyz - input.world_pos.xyz);
        float3 reflectDir = reflect(-lightDir, norm);
        // 32 to be replaced with material shininess
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        float3 specular = pointLights.specular.xyz * spec * ObjTexture.Sample(ObjSamplerState, input.UV).rgb;
        float distance = length(pointLights.pos.xyz - input.world_pos.xyz);
        float attenuation = 1.0f / (pointLights.constant_lin_quad.x + pointLights.constant_lin_quad.y * distance + pointLights.constant_lin_quad.z * (distance * distance));
        diffuse *= attenuation;
        specular *= attenuation;
        result += diffuse;
        result += specular;
    }
    return float4(result,1.0f);

}
