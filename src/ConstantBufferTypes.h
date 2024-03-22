#pragma once

#include <glm/glm.hpp>

struct ConstantBuffer
{

};

struct ConstantBufferPerObject
{
    glm::mat4 PVM;
    glm::mat4 world;
};

struct ConstantBufferSkybox : public ConstantBuffer
{
    glm::mat4 PV_no_translation;
};
