#pragma once

#include <glm/glm.hpp>

#include "AK/Types.h"

i32 constexpr MAX_POINT_LIGHTS = 20;
i32 constexpr MAX_SPOT_LIGHTS = 20;

struct ConstantBuffer
{
};

struct DXWave
{
    glm::vec2 direction;
    glm::vec2 padding;

    float speed;
    float steepness;
    float wave_length;
    float amplitude;
};

struct ConstantBufferWave
{
    DXWave waves[15];
    float time;
    int number_of_waves;
};

struct DXPointLight
{
    glm::vec3 position;
    float padding1;
    glm::vec3 ambient;
    float padding2;
    glm::vec3 diffuse;
    float padding3;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float far_plane;
    float near_plane;
};

struct DXSpotLight
{
    glm::vec3 position;
    float scattering_factor;
    glm::vec3 direction;

    float cut_off;
    float outer_cut_off;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    float far_plane;
    glm::vec3 diffuse;
    float near_plane;
    glm::vec3 specular;
    float padding6;

    glm::mat4 light_projection_view;
    glm::mat4 inv_light_model;
    glm::mat4 light_model;

    u32 blocker_search_num_samples;
    u32 pcf_num_samples;
    float light_world_size;
    float light_frustum_width;
};

struct DXDirectionalLight
{
    glm::vec3 direction;
    float padding1;
    glm::vec3 ambient;
    float padding2;
    glm::vec3 diffuse;
    float padding3;
    glm::vec3 specular;
    float padding4;

    glm::mat4 light_projection_view;

    u32 blocker_search_num_samples;
    u32 pcf_num_samples;
    float light_world_size;
    float light_frustum_width;
    float near_plane;
    glm::vec3 padding5;
};

struct ConstantBufferPerObject
{
    glm::mat4 projection_view_model;
    glm::mat4 model;
    glm::mat4 projection_view;
    i32 is_glowing;
};

struct ConstantBufferParticle
{
    glm::vec4 color;
};

struct ConstantBufferCameraPosition
{
    glm::vec3 camera_pos;
    float padding1;
};

struct ConstantBufferLight
{
    DXDirectionalLight directional_light;
    DXPointLight point_lights[MAX_POINT_LIGHTS];
    DXSpotLight spot_lights[MAX_SPOT_LIGHTS];

    glm::vec3 camera_pos;
    i32 number_of_point_lights;
    i32 number_of_spot_lights;
    float gamma;
    float exposure;
    float padding;
};

struct ConstantBufferSkybox : public ConstantBuffer
{
    glm::mat4 PV_no_translation;
};

struct ConstantBufferDepth : public ConstantBuffer
{
    glm::vec3 light_pos;
    float far_plane;
};

struct ConstantBufferPSMisc : public ConstantBuffer
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec2 mouse_pos;
    float light_range;
    float time;
    bool is_fog_rendered;
};

struct ConstantBufferSSAO
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec4 kernel_samples[64];
};

struct ConstantBufferWater
{
    glm::vec4 top_color;
    glm::vec4 bottom_color;

    float normalmap_scroll_speed_0;
    float normalmap_scroll_speed_1;

    float normalmap_scale0;
    float normalmap_scale1;

    float combined_amplitude;
    float phong_contribution;
};
