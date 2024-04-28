#include "DirectionalLight.h"

#include "Entity.h"
#include "Renderer.h"

std::shared_ptr<DirectionalLight> DirectionalLight::create()
{
    auto directional_light = std::make_shared<DirectionalLight>(AK::Badge<DirectionalLight> {});
    return directional_light;
}

void DirectionalLight::draw_editor()
{
    Light::draw_editor();
}

glm::mat4 DirectionalLight::get_projection_view_matrix()
{
    if (m_last_model_matrix != entity->transform->get_model_matrix() && entity != nullptr)
    {
        m_last_model_matrix = entity->transform->get_model_matrix();
        glm::mat4 const projection_matrix = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, -entity->transform->get_position().y, entity->transform->get_position().y);
        glm::mat4 const view_matrix = glm::lookAt(entity->transform->get_position(), entity->transform->get_position() + entity->transform->get_forward(), entity->transform->get_up());
        m_projection_view_matrix = projection_matrix * view_matrix;
    }

    return m_projection_view_matrix;
}
