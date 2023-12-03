#include "DirectionalLight.h"

#include "Renderer.h"

std::shared_ptr<DirectionalLight> DirectionalLight::create()
{
    auto directional_light = std::make_shared<DirectionalLight>();
    Renderer::get_instance()->register_light(directional_light);
    return directional_light;
}
