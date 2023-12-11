#include "Light.h"

#include "Renderer.h"

std::shared_ptr<Light> Light::create()
{
    auto light = std::make_shared<Light>();
    Renderer::get_instance()->register_light(light);
    return light;
}

std::string Light::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}
