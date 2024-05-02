#include "PointLight.h"

#include "Renderer.h"

std::shared_ptr<PointLight> PointLight::create()
{
    auto point_light = std::make_shared<PointLight>(AK::Badge<PointLight> {});
    Renderer::get_instance()->register_light(point_light);
    point_light->set_up_shadow_mapping();
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

void PointLight::set_up_shadow_mapping()
{

}
