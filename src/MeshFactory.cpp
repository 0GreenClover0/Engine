#include "MeshFactory.h"

#include "MeshGL.h"
#include "Renderer.h"

std::shared_ptr<Mesh> MeshFactory::create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
                                          std::vector<Texture> const& textures, DrawType const draw_type,
                                          std::shared_ptr<Material> const& material, DrawFunctionType const draw_function)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        auto mesh = std::make_shared<MeshGL>(AK::Badge<MeshFactory> {}, vertices, indices, textures, draw_type, material, draw_function);
        return mesh;
    }

    std::unreachable();
}
