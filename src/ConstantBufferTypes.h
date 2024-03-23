#pragma once

#include <glm/glm.hpp>

struct ConstantBuffer
{

};

struct ConstantBufferPerObject
{
    glm::mat4 projection_view;
};

struct ConstantBufferSkybox : public ConstantBuffer
{
    glm::mat4 PV_no_translation;
};
