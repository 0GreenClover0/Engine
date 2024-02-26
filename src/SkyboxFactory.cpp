#include "SkyboxFactory.h"

#include "Renderer.h"
#include "SkyboxGL.h"

std::shared_ptr<Skybox> SkyboxFactory::create(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        return std::make_shared<SkyboxGL>(AK::Badge<SkyboxFactory> {}, material, face_paths);
    }

    std::unreachable();
}
