struct PCSSSettingsPerLight
{
    int blocker_search_num_samples;
    int pcf_num_samples;
    float light_world_size;
    float light_frustum_width;
};

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

    // PCSS variables
    PCSSSettingsPerLight pcss_settings;
    float near_plane;
};

struct SpotLight
{
    float3 position;
    float scattering_factor;
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
    float4x4 inv_model;
    float4x4 model;
    PCSSSettingsPerLight pcss_settings;
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
