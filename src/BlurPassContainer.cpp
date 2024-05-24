#include "BlurPassContainer.h"

#include <glm/gtc/type_ptr.inl>

#include "RendererDX11.h"
#include "ResourceManager.h"

std::shared_ptr<BlurPassContainer> BlurPassContainer::create()
{
    return std::make_shared<BlurPassContainer>(AK::Badge<BlurPassContainer> {});
}

BlurPassContainer::BlurPassContainer(AK::Badge<BlurPassContainer>)
{
    m_pass_shader = ResourceManager::get_instance().load_shader("./res/shaders/blur.hlsl", "./res/shaders/blur.hlsl");

    update();
}

void BlurPassContainer::clear_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->ClearRenderTargetView(m_blur_render_target, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
}

void BlurPassContainer::bind_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->OMSetRenderTargets(1, &m_blur_render_target, nullptr);
}

void BlurPassContainer::bind_shader_resources() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->PSSetShaderResources(14, 1, &m_blur_srv);
}

void BlurPassContainer::update()
{
    auto renderer = RendererDX11::get_instance_dx11();

    if (m_blurred_texture != nullptr)
    {
        m_blurred_texture->Release();
    }

    D3D11_TEXTURE2D_DESC blur_tex_desc = {};
    blur_tex_desc.Width = renderer->get_main_view_port().Width;
    blur_tex_desc.Height = renderer->get_main_view_port().Height;
    blur_tex_desc.MipLevels = 1;
    blur_tex_desc.ArraySize = 1;
    blur_tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    blur_tex_desc.SampleDesc.Count = 1;
    blur_tex_desc.SampleDesc.Quality = 0;
    blur_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    blur_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    blur_tex_desc.CPUAccessFlags = 0;
    blur_tex_desc.MiscFlags = 0;

    HRESULT hr = renderer->get_device()->CreateTexture2D(&blur_tex_desc, nullptr, &m_blurred_texture);
    assert(SUCCEEDED(hr));

    if (m_blur_srv != nullptr)
    {
        m_blur_srv->Release();
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC blur_srv_desc = {};
    blur_srv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    blur_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    blur_srv_desc.Texture2D.MostDetailedMip = 0;
    blur_srv_desc.Texture2D.MipLevels = 1;

    hr = renderer->get_device()->CreateShaderResourceView(m_blurred_texture, &blur_srv_desc, &m_blur_srv);
    assert(SUCCEEDED(hr));

    if (m_blur_render_target != nullptr)
    {
        m_blur_render_target->Release();
    }

    D3D11_RENDER_TARGET_VIEW_DESC blur_rtv_desc = {};
    blur_rtv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    blur_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    blur_rtv_desc.Texture2D.MipSlice = 0;

    hr = renderer->get_device()->CreateRenderTargetView(m_blurred_texture, &blur_rtv_desc, &m_blur_render_target);
    assert(SUCCEEDED(hr));
}
