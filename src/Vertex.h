#pragma once

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture_coordinates;
    glm::ivec4 skin_indices = {-1, -1, -1, -1};
    glm::vec4 skin_weights = {0, 0, 0, 0};
};
