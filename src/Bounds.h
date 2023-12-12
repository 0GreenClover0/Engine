#pragma once
#include <glm/vec3.hpp>

struct Bounds
{
    glm::vec3 min = {};
    float min_array[3];
    glm::vec3 max = {};
    float max_array[3];
};
