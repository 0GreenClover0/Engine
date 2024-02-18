#include "DirectionalLight.h"

#include <imgui.h>

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
