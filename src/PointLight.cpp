#include "PointLight.h"

#include "Renderer.h"

std::shared_ptr<PointLight> PointLight::create()
{
    auto point_light = std::make_shared<PointLight>(AK::Badge<PointLight> {});
    return point_light;
}

void PointLight::draw_editor()
{
    Light::draw_editor();
}
