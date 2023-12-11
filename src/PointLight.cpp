#include "PointLight.h"

#include "Renderer.h"

std::shared_ptr<PointLight> PointLight::create()
{
    auto point_light = std::make_shared<PointLight>();
    Renderer::get_instance()->register_light(point_light);
    return point_light;
}

void PointLight::draw_editor()
{
    Light::draw_editor();
}

std::string PointLight::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}
