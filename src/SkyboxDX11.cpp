#include "SkyboxDX11.h"
#include "RendererDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureLoaderDX11.h"
#include "Entity.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "SkyboxFactory.h"

SkyboxDX11::SkyboxDX11(AK::Badge<SkyboxFactory>, std::shared_ptr<Material> const& material, std::string const& path) : Skybox(material, path)
{
    create_cube();

    m_texture = ResourceManager::get_instance().load_cubemap(path, TextureType::Diffuse);
}

void SkyboxDX11::bind()
{
    RendererDX11::get_instance_dx11()->get_device_context()->PSSetShaderResources(10, 1, &m_texture->shader_resource_view);
}

void SkyboxDX11::unbind()
{
    ID3D11ShaderResourceView* null_shader_resource_view = nullptr;
    RendererDX11::get_instance_dx11()->get_device_context()->PSSetShaderResources(10, 1, &null_shader_resource_view);
}

void SkyboxDX11::draw() const
{
    if (!get_can_tick())
        return;

    // preferably in the update() method, but for some reason it doesn't work even when playing
    entity->transform->set_local_position(Camera::get_main_camera()->get_position());

    bind_texture();

    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();

    u32 constexpr offset = 0;
    device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetVertexBuffers(0, 1, m_vertex_buffer->get_address_of(), m_vertex_buffer->stride_ptr(), &offset);
    device_context->IASetIndexBuffer(m_index_buffer->get(), DXGI_FORMAT_R32_UINT, 0);
    device_context->DrawIndexed(m_index_buffer->buffer_size(), 0, 0);

    unbind_texture();
}

void SkyboxDX11::update()
{
    entity->transform->set_local_position(Camera::get_main_camera()->get_position());
}

void SkyboxDX11::awake()
{
    set_can_tick(true);
}

void SkyboxDX11::bind_texture() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    auto const device_context = renderer->get_device_context();

    device_context->PSSetShaderResources(0, 1, &m_texture->shader_resource_view);
    device_context->PSSetSamplers(0, 1, &m_texture->image_sampler_state);
    device_context->OMSetDepthStencilState(renderer->get_depth_stencil_state(), 0);
}

void SkyboxDX11::unbind_texture() const
{
    ID3D11ShaderResourceView* null_shader_resource_view = nullptr;
    ID3D11SamplerState* null_sampler_state = nullptr;

    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();
    device_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
    device_context->PSSetSamplers(0, 1, &null_sampler_state);
    device_context->OMSetDepthStencilState(nullptr, 0);
}

void SkyboxDX11::create_cube()
{
    auto device = RendererDX11::get_instance_dx11()->get_device();

    std::vector<u32> indices = {
        0, 1, 2, 2, 3, 0, // Front face
        1, 5, 6, 6, 2, 1, // Right face
        5, 4, 7, 7, 6, 5, // Back face
        4, 0, 3, 3, 7, 4, // Left face
        3, 2, 6, 6, 7, 3, // Top face
        4, 5, 1, 1, 0, 4  // Bottom face
    };

    std::vector<Vertex> vertices = {};
    vertices.reserve(8);

    float constexpr size = 1.0f;
    std::array constexpr corners = {
        glm::vec3(-size, -size, -size),
        glm::vec3(size, -size, -size),
        glm::vec3(size, size, -size),
        glm::vec3(-size, size, -size),
        glm::vec3(-size, -size, size),
        glm::vec3(size, -size, size),
        glm::vec3(size, size, size),
        glm::vec3(-size, size, size)
    };

    for (auto const corner : corners)
    {
        Vertex vertex = {};
        vertex.position = corner;
        vertex.normal = glm::vec3(0.0f);
        vertex.texture_coordinates = glm::vec2(0.0f);
        vertices.push_back(vertex);
    }

    m_vertex_buffer = std::make_shared<VertexBufferDX11>(device, vertices.data(), vertices.size());
    m_index_buffer = std::make_shared<IndexBufferDX11>(device, indices.data(), indices.size());
}
