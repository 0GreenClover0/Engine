#include "Light.h"

#include "Renderer.h"

std::shared_ptr<Light> Light::create()
{
    auto light = std::make_shared<Light>();
    Renderer::get_instance()->register_light(light);
    return light;
}
