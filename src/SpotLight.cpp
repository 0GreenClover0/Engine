#include "SpotLight.h"

#include "Renderer.h"

std::shared_ptr<SpotLight> SpotLight::create()
{
    auto spot_light = std::make_shared<SpotLight>();
    Renderer::get_instance()->register_light(spot_light);
    return spot_light;
}

std::string SpotLight::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}
