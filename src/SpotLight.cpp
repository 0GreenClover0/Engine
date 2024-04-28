#include "SpotLight.h"

#include <imgui.h>

#include "Renderer.h"

std::shared_ptr<SpotLight> SpotLight::create()
{
    auto spot_light = std::make_shared<SpotLight>(AK::Badge<SpotLight> {});
    Renderer::get_instance()->register_light(spot_light);
    return spot_light;
}

void SpotLight::draw_editor()
{
    Light::draw_editor();
}
