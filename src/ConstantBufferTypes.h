#pragma once

#include <glm/glm.hpp>

struct ConstantBuffer
{

};
struct DXPointLight {
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 constant_linear_quadratic;

};
struct ConstantBufferPerObject
{
    glm::mat4 projection_view;
    glm::mat4 model;
};

struct ConstantBufferLight
{
    DXPointLight point_lights;
    glm::vec4 camera_pos;
};


struct ConstantBufferSkybox : public ConstantBuffer
{
    glm::mat4 PV_no_translation;
};
