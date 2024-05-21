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
    float4x4 projection_view;
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
    float far_plane;
    float3 diffuse;
    float near_plane;
    float3 specular;

    float4x4 projection_view;
    float4x4 model;
};

struct Wave
{
    float2 direction;

    float padding;
    float padding2;

    float speed;
    float steepness;
    float wave_length;
    float amplitude;
};
