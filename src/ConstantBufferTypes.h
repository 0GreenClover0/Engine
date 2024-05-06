#pragma once

#include <glm/glm.hpp>

i32 constexpr MAX_POINT_LIGHTS = 4;
i32 constexpr MAX_SPOT_LIGHTS = 4;

struct ConstantBuffer
{

};

struct DXWave
{
    // first 16 byte block
    glm::vec2 direction;
    glm::vec2 padding;
    //second block
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
    float padding1;
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

    glm::mat4 light_projection_view_model;
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
};

struct ConstantBufferPerObject
{
    glm::mat4 projection_view_model;
    glm::mat4 model;
    glm::mat4 light_projection_view_model;
};

struct ConstantBufferLight
{
    DXDirectionalLight directional_light;
    DXPointLight point_lights[MAX_POINT_LIGHTS];
    DXSpotLight spot_lights[MAX_SPOT_LIGHTS];

    glm::vec3 camera_pos;
    i32 number_of_point_lights;
    i32 number_of_spot_lights;
    glm::vec3 padding;
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
