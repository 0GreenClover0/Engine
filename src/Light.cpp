#include "Light.h"

#include <imgui.h>

#include "Renderer.h"

std::shared_ptr<Light> Light::create()
{
    auto light = std::make_shared<Light>(AK::Badge<Light> {});
    Renderer::get_instance()->register_light(light);
    return light;
}

void Light::draw_editor()
{
    Component::draw_editor();

    float diffuse_color[] = { diffuse.x, diffuse.y, diffuse.z };
    ImGui::ColorEdit3("Diffuse color", diffuse_color);
    diffuse = glm::vec3(diffuse_color[0], diffuse_color[1], diffuse_color[2]);

    float ambient_color[] = { ambient.x, ambient.y, ambient.z };
    ImGui::ColorEdit3("Ambient color", ambient_color);
    ambient = glm::vec3(ambient_color[0], ambient_color[1], ambient_color[2]);

    float specular_color[] = { specular.x, specular.y, specular.z };
    ImGui::ColorEdit3("Specular color", specular_color);
    specular = glm::vec3(specular_color[0], specular_color[1], specular_color[2]);
}

std::string Light::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Light::on_enabled()
{
    Renderer::get_instance()->register_light(std::static_pointer_cast<Light>(shared_from_this()));
}

void Light::on_disabled()
{
    Renderer::get_instance()->unregister_light(std::static_pointer_cast<Light>(shared_from_this()));
}
