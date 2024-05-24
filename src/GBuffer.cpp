#include "GBuffer.h"

#include <glm/gtc/type_ptr.inl>

#include "RendererDX11.h"
#include "ResourceManager.h"

std::shared_ptr<GBuffer> GBuffer::create()
{
    return std::make_shared<GBuffer>(AK::Badge<GBuffer> {});
}

GBuffer::GBuffer(AK::Badge<GBuffer>)
{
    m_pass_shader = ResourceManager::get_instance().load_shader("./res/shaders/g_buffer_shader.hlsl", "./res/shaders/g_buffer_shader.hlsl");

    update();
}

void GBuffer::clear_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();

    for (auto const m_gbuffer_rendertarget : m_gbuffer_rendertargets)
    {
        renderer->get_device_context()->ClearRenderTargetView(m_gbuffer_rendertarget, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    }
}

void GBuffer::bind_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->OMSetRenderTargets(m_gbuffer_rendertargets.size(), m_gbuffer_rendertargets.data(),
                                                       renderer->get_depth_stencil_view());
}

void GBuffer::bind_shader_resources() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->PSSetShaderResources(10, 1, &m_position_texture_view);
    renderer->get_device_context()->PSSetShaderResources(11, 1, &m_normal_texture_view);
    renderer->get_device_context()->PSSetShaderResources(12, 1, &m_diffuse_texture_view);
}

void GBuffer::update()
{
    auto renderer = RendererDX11::get_instance_dx11();
    auto viewport = renderer->get_main_view_port();

    if (m_diffuse_texture != nullptr)
    {
        m_diffuse_texture->Release();
    }

    // Create the render target view
    D3D11_TEXTURE2D_DESC diffuse_tex_desc = {};
    diffuse_tex_desc.Width = static_cast<u32>(viewport.Width);
    diffuse_tex_desc.Height = static_cast<u32>(viewport.Height);
    diffuse_tex_desc.MipLevels = 1;
    diffuse_tex_desc.ArraySize = 1;
    diffuse_tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    diffuse_tex_desc.SampleDesc.Count = 1;
    diffuse_tex_desc.SampleDesc.Quality = 0;
    diffuse_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    diffuse_tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    diffuse_tex_desc.CPUAccessFlags = 0;
    diffuse_tex_desc.MiscFlags = 0;

    HRESULT hr = renderer->get_device()->CreateTexture2D(&diffuse_tex_desc, nullptr, &m_diffuse_texture);
    assert(SUCCEEDED(hr));

    if (m_normal_texture != nullptr)
    {
        m_normal_texture->Release();
    }

    D3D11_TEXTURE2D_DESC normal_tex_desc = diffuse_tex_desc;
    normal_tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    hr = renderer->get_device()->CreateTexture2D(&normal_tex_desc, nullptr, &m_normal_texture);
    assert(SUCCEEDED(hr));

    if (m_position_texture != nullptr)
    {
        m_position_texture->Release();
    }

    D3D11_TEXTURE2D_DESC position_tex_desc = normal_tex_desc;

    hr = renderer->get_device()->CreateTexture2D(&position_tex_desc, nullptr, &m_position_texture);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = position_tex_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;

    if (m_position_texture_view != nullptr)
    {
        m_position_texture_view->Release();
    }

    hr = renderer->get_device()->CreateShaderResourceView(m_position_texture, &srv_desc, &m_position_texture_view);
    assert(SUCCEEDED(hr));

    if (m_normal_texture_view != nullptr)
    {
        m_normal_texture_view->Release();
    }

    srv_desc.Format = normal_tex_desc.Format;

    hr = renderer->get_device()->CreateShaderResourceView(m_normal_texture, &srv_desc, &m_normal_texture_view);
    assert(SUCCEEDED(hr));

    srv_desc.Format = diffuse_tex_desc.Format;

    if (m_diffuse_texture_view != nullptr)
    {
        m_diffuse_texture_view->Release();
    }

    hr = renderer->get_device()->CreateShaderResourceView(m_diffuse_texture, &srv_desc, &m_diffuse_texture_view);
    assert(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = position_tex_desc.Format;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtv_desc.Texture2D.MipSlice = 0;

    for (auto const& render_target : m_gbuffer_rendertargets)
    {
        if (render_target != nullptr)
        {
            render_target->Release();
        }
    }

    hr = renderer->get_device()->CreateRenderTargetView(m_position_texture, &rtv_desc, &m_gbuffer_rendertargets[0]);
    assert(SUCCEEDED(hr));

    rtv_desc.Format = normal_tex_desc.Format;

    hr = renderer->get_device()->CreateRenderTargetView(m_normal_texture, &rtv_desc, &m_gbuffer_rendertargets[1]);
    assert(SUCCEEDED(hr));

    rtv_desc.Format = diffuse_tex_desc.Format;

    hr = renderer->get_device()->CreateRenderTargetView(m_diffuse_texture, &rtv_desc, &m_gbuffer_rendertargets[2]);
    assert(SUCCEEDED(hr));
}
