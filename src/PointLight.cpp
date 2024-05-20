#include "PointLight.h"

#include "Entity.h"
#include "Renderer.h"
#include "RendererDX11.h"

#include <d3d11.h>
#include <glm/glm.hpp>

std::shared_ptr<PointLight> PointLight::create()
{
    auto point_light = std::make_shared<PointLight>(AK::Badge<PointLight> {});
    point_light->set_up_shadow_mapping();
    return point_light;
}

void PointLight::draw_editor()
{
    Light::draw_editor();
    ImGui::Text("Attenuation:");
    ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f);
}

void PointLight::set_up_shadow_mapping()
{
    auto renderer = RendererDX11::get_instance_dx11();
    D3D11_TEXTURE2D_DESC shadow_texture_desc = {};
    shadow_texture_desc.Width = static_cast<u32>(renderer->SHADOW_MAP_SIZE);
    shadow_texture_desc.Height = static_cast<u32>(renderer->SHADOW_MAP_SIZE);
    shadow_texture_desc.MipLevels = 1;
    shadow_texture_desc.ArraySize = 6;
    shadow_texture_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadow_texture_desc.SampleDesc.Count = 1;
    shadow_texture_desc.SampleDesc.Quality = 0;
    shadow_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    shadow_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    shadow_texture_desc.CPUAccessFlags = 0;
    shadow_texture_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    HRESULT hr = renderer->get_device()->CreateTexture2D(&shadow_texture_desc, nullptr, &m_shadow_texture);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shadow_shader_resource_view_desc = {};
    shadow_shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shadow_shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    shadow_shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
    shadow_shader_resource_view_desc.Texture2D.MipLevels = shadow_texture_desc.MipLevels;

    hr = renderer->get_device()->CreateShaderResourceView(m_shadow_texture, &shadow_shader_resource_view_desc,
                                                          &m_shadow_shader_resource_view);
    assert(SUCCEEDED(hr));

    for (u32 i = 0; i < 6; i++)
    {
        ID3D11DepthStencilView* shadow_depth_stencil_view = nullptr;
        D3D11_DEPTH_STENCIL_VIEW_DESC shadow_depth_stencil_view_desc = {};
        shadow_depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        shadow_depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        shadow_depth_stencil_view_desc.Texture2DArray.MipSlice = 0;
        shadow_depth_stencil_view_desc.Texture2DArray.ArraySize = 1;
        shadow_depth_stencil_view_desc.Texture2DArray.MipSlice = 0;
        shadow_depth_stencil_view_desc.Texture2DArray.FirstArraySlice = i;
        shadow_depth_stencil_view_desc.Flags = 0;

        hr = renderer->get_device()->CreateDepthStencilView(m_shadow_texture, &shadow_depth_stencil_view_desc, &shadow_depth_stencil_view);
        assert(SUCCEEDED(hr));

        m_shadow_depth_stencil_views.emplace_back(shadow_depth_stencil_view);
    }
}

void PointLight::set_render_target_for_shadow_mapping(u32 const face_index) const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->OMSetRenderTargets(1, &renderer->g_emptyRenderTargetView, m_shadow_depth_stencil_views[face_index]);
    renderer->get_device_context()->ClearDepthStencilView(m_shadow_depth_stencil_views[face_index], D3D11_CLEAR_DEPTH, 1.0f, 0);
}

glm::mat4 PointLight::get_projection_view_matrix(u32 const face_index)
{
    if (entity->transform->get_model_matrix() != m_last_model_matrix)
    {
        update_pv_matrices();
    }

    return m_projection_view_matrices[face_index];
}

void PointLight::update_pv_matrices()
{
    auto const renderer = RendererDX11::get_instance_dx11();

    m_last_model_matrix = entity->transform->get_model_matrix();

    auto const transform = entity->transform;

    float const aspect_ratio = renderer->SHADOW_MAP_SIZE / renderer->SHADOW_MAP_SIZE;

    glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect_ratio, m_near_plane, m_far_plane);

    // I hate to say it, but I'm not sure why this line is necessary, could be something related to using glm with DirectX
    shadow_proj = glm::scale(shadow_proj, glm::vec3(1.0f, -1.0f, 1.0f));

    glm::vec3 const light_pos = transform->get_position();

    m_projection_view_matrices[0] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));

    m_projection_view_matrices[1] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));

    m_projection_view_matrices[2] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));

    m_projection_view_matrices[3] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));

    m_projection_view_matrices[4] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));

    m_projection_view_matrices[5] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}
