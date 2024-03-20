#include "MeshFactory.h"

#include "MeshDX11.h"
#include "MeshGL.h"
#include "Renderer.h"

std::shared_ptr<Mesh> MeshFactory::create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
                                          std::vector<Texture> const& textures, DrawType const draw_type,
                                          std::shared_ptr<Material> const& material, DrawFunctionType const draw_function)
{
    switch (Renderer::renderer_api)
    {
    case Renderer::RendererApi::OpenGL:
        {
            auto mesh = std::make_shared<MeshGL>(AK::Badge<MeshFactory> {}, vertices, indices, textures, draw_type, material, draw_function);
            return mesh;
        }

    case Renderer::RendererApi::DirectX11:
        {
            auto mesh = std::make_shared<MeshDX11>(AK::Badge<MeshFactory> {}, vertices, indices, textures, draw_type, material, draw_function);
            return mesh;
        }

    default:
        std::unreachable();
    }
}
