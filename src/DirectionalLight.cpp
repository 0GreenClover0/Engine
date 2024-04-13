#include "DirectionalLight.h"

#include "Entity.h"
#include "Renderer.h"

std::shared_ptr<DirectionalLight> DirectionalLight::create()
{
    auto directional_light = std::make_shared<DirectionalLight>(AK::Badge<DirectionalLight> {});
    Renderer::get_instance()->register_light(directional_light);
    return directional_light;
}

void DirectionalLight::draw_editor()
{
    Light::draw_editor();
}

std::string DirectionalLight::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
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
