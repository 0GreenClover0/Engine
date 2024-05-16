#include "SSAO.h"

#include <random>

#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.inl>

#include "AK/Badge.h"
#include "RendererDX11.h"
#include "ResourceManager.h"

std::shared_ptr<SSAO> SSAO::create()
{
    return std::make_shared<SSAO>(AK::Badge<SSAO> {});
}

SSAO::SSAO(AK::Badge<SSAO>)
{
    m_pass_shader = ResourceManager::get_instance().load_shader("./res/shaders/ssao.hlsl", "./res/shaders/ssao.hlsl");

    update();
}

void SSAO::clear_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->ClearRenderTargetView(m_ssao_render_target, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
}

void SSAO::bind_render_targets() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->OMSetRenderTargets(1, &m_ssao_render_target, nullptr);
    renderer->get_device_context()->PSSetShaderResources(13, 1, &m_ssao_kernel_rotations_srv);
}

void SSAO::bind_shader_resources() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->PSSetShaderResources(13, 1, &m_ssao_kernel_rotations_srv);
    renderer->get_device_context()->PSSetShaderResources(14, 1, &m_ssao_srv);
}

void SSAO::update()
{
    std::uniform_real_distribution random_floats(0.0f, 1.0f);
    std::default_random_engine generator = {};

    for (u32 i = 0; i < kernel_size; ++i)
    {
        glm::vec4 sample(
            random_floats(generator) * 2.0f - 1.0f,
            random_floats(generator) * 2.0f - 1.0f,
            random_floats(generator),
            0.0f
        );

        sample = glm::normalize(sample);
        sample *= random_floats(generator);

        float scale = static_cast<float>(i) / static_cast<float>(kernel_size);
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        m_ssao_kernel[i] = sample;
    }

    std::array<glm::vec3, noise_size> ssao_noise = {};
    for (u32 i = 0; i < noise_size; ++i)
    {
        ssao_noise[i] = {random_floats(generator) * 2.0f - 1.0f, random_floats(generator) * 2.0f - 1.0f,0.0f};
    }

    D3D11_TEXTURE2D_DESC rotations_tex_desc = {};
    rotations_tex_desc.Width = 4;
    rotations_tex_desc.Height = 4;
    rotations_tex_desc.MipLevels = 1;
    rotations_tex_desc.ArraySize = 1;
    rotations_tex_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rotations_tex_desc.SampleDesc.Count = 1;
    rotations_tex_desc.SampleDesc.Quality = 0;
    rotations_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    rotations_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    rotations_tex_desc.CPUAccessFlags = 0;

    auto renderer = RendererDX11::get_instance_dx11();
    HRESULT hr = renderer->get_device()->CreateTexture2D(&rotations_tex_desc, nullptr, &m_ssao_kernel_rotations_buffer);
    assert(SUCCEEDED(hr));

    D3D11_TEXTURE2D_DESC ssao_tex_desc = {};
    ssao_tex_desc.Width = static_cast<u32>(renderer->get_main_view_port().Width);
    ssao_tex_desc.Height = static_cast<u32>(renderer->get_main_view_port().Height);
    ssao_tex_desc.MipLevels = 1;
    ssao_tex_desc.ArraySize = 1;
    ssao_tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ssao_tex_desc.SampleDesc.Count = 1;
    ssao_tex_desc.SampleDesc.Quality = 0;
    ssao_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    ssao_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    ssao_tex_desc.CPUAccessFlags = 0;
    ssao_tex_desc.MiscFlags = 0;

    hr = renderer->get_device()->CreateTexture2D(&ssao_tex_desc, nullptr, &m_ssao_texture);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;

    hr = renderer->get_device()->CreateShaderResourceView(m_ssao_texture, &srv_desc, &m_ssao_srv);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC rotations_srv_desc = {};
    rotations_srv_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rotations_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    rotations_srv_desc.Texture2D.MostDetailedMip = 0;
    rotations_srv_desc.Texture2D.MipLevels = 1;

    hr = renderer->get_device()->CreateShaderResourceView(m_ssao_kernel_rotations_buffer, &rotations_srv_desc, &m_ssao_kernel_rotations_srv);
    assert(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtv_desc.Texture2D.MipSlice = 0;

    hr = renderer->get_device()->CreateRenderTargetView(m_ssao_texture, &rtv_desc, &m_ssao_render_target);
    assert(SUCCEEDED(hr));
}

std::array<glm::vec4, SSAO::kernel_size> SSAO::get_ssao_kernel() const
{
    return m_ssao_kernel;
}
