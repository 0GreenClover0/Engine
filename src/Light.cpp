#include "Light.h"

#include "Renderer.h"

#if EDITOR
#include <imgui.h>
#endif

Light::~Light()
{
}

#if EDITOR
void Light::draw_editor()
{
    Component::draw_editor();

    float diffuse_color[] = {diffuse.x, diffuse.y, diffuse.z};
    ImGui::ColorEdit3("Diffuse color", diffuse_color);
    diffuse = glm::vec3(diffuse_color[0], diffuse_color[1], diffuse_color[2]);

    float ambient_color[] = {ambient.x, ambient.y, ambient.z};
    ImGui::ColorEdit3("Ambient color", ambient_color);
    ambient = glm::vec3(ambient_color[0], ambient_color[1], ambient_color[2]);

    float specular_color[] = {specular.x, specular.y, specular.z};
    ImGui::ColorEdit3("Specular color", specular_color);
    specular = glm::vec3(specular_color[0], specular_color[1], specular_color[2]);
}
#endif

void Light::initialize()
{
    Renderer::get_instance()->register_light(std::dynamic_pointer_cast<Light>(shared_from_this()));
}

void Light::uninitialize()
{
    Renderer::get_instance()->unregister_light(std::dynamic_pointer_cast<Light>(shared_from_this()));
}

void Light::on_enabled()
{
    auto const light = std::dynamic_pointer_cast<Light>(shared_from_this());

    // Light might already be registered in initialize() or create() methods
    if (!Renderer::get_instance()->is_light_registered(light))
    {
        Renderer::get_instance()->register_light(light);
    }
}

void Light::on_disabled()
{
    Renderer::get_instance()->unregister_light(std::dynamic_pointer_cast<Light>(shared_from_this()));
}

ID3D11ShaderResourceView* const* Light::get_shadow_shader_resource_view_address() const
{
    return &m_shadow_shader_resource_view;
}

ID3D11ShaderResourceView* Light::get_shadow_shader_resource_view() const
{
    return m_shadow_shader_resource_view;
}
