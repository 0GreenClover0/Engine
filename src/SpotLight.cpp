#include "SpotLight.h"

#include "Renderer.h"

std::shared_ptr<SpotLight> SpotLight::create()
{
    auto spot_light = std::make_shared<SpotLight>();
    Renderer::get_instance()->register_light(spot_light);
    return spot_light;
}
