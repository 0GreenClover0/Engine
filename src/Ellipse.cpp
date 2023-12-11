#include "Ellipse.h"

#include <glm/trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>

Ellipse::Ellipse(float center_x, float center_z, float radius_x, float radius_z, int segment_count, std::shared_ptr<Material> const& material)
    : Model(material), center_x(center_x), center_z(center_z), radius_x(radius_x), radius_z(radius_z), segment_count(segment_count)
{
    draw_type = GL_LINE_LOOP;
    meshes.emplace_back(create_ellipse());
}

std::string Ellipse::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

Mesh Ellipse::create_ellipse() const
{ 
    float const theta = 2 * glm::pi<float>() / static_cast<float>(segment_count); 
    float const c = glm::cos(theta);
    float const s = glm::sin(theta);

    float x = 1; // Start at angle = 0 
    float z = 0;

    std::vector<Vertex> vertices;

    glBegin(GL_LINE_LOOP); 
    for (int i = 0; i < segment_count; ++i) 
    {
        Vertex vertex = {};
        // Apply radius and offset
        vertex.position = glm::vec3(x * radius_x + center_x, 0.0f, z * radius_z + center_z);

        // Apply the rotation matrix
        float const t = x;
        x = c * x - s * z;
        z = s * t + c * z;

        vertices.emplace_back(vertex);
    } 
    glEnd();

    return Mesh::create(vertices, {}, {}, draw_type, material);
}
