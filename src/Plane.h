#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

struct Plane
{
    glm::vec3 normal = {};
    float distance = 0.0f;

    Plane() = default;

    Plane(glm::vec3 const& normal, glm::vec3 const& point) : normal(normal), distance(-glm::dot(normal, point))
    {
    }

    Plane(glm::vec3 const& normal, float const distance) : normal(normal), distance(distance)
    {
    }

    [[nodiscard]] std::string to_string() const
    {
        return glm::to_string(normal) + " " + std::to_string(distance);
    }
};
