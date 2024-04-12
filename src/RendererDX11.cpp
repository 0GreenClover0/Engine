#include "RendererDX11.h"

#include <iostream>
#include "TextureLoaderDX11.h"
#include "Drawable.h"
#include "Entity.h"
#include "Model.h"
#include "Camera.h"

#include <array>

std::shared_ptr<RendererDX11> RendererDX11::create()
{
    auto renderer = std::make_shared<RendererDX11>(AK::Badge<RendererDX11> {});

    assert(m_instance == nullptr);

    set_instance(renderer);
    set_instance_dx11(renderer);

    if (!renderer->create_device_d3d(Engine::window->get_win32_window()))
    {
        std::cout << "DirectX11: Error occurred while creating a d3d device." << "\n";
    }

    TextureLoaderDX11::create();

    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = static_cast<UINT>(sizeof(ConstantBufferPerObject) + (16 - (sizeof(ConstantBufferPerObject) % 16)));
    desc.StructureByteStride = 0;

    HRESULT hr = renderer->get_device()->CreateBuffer(&desc, nullptr, &renderer->m_constant_buffer_per_object);

    assert(SUCCEEDED(hr));

    glfwSetWindowSizeCallback(Engine::window->get_glfw_window(), on_window_resize);

    D3D11_BUFFER_DESC light_buffer_desc = {};
    light_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    light_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    light_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    light_buffer_desc.MiscFlags = 0;
    light_buffer_desc.ByteWidth = sizeof(ConstantBufferLight);

    hr = renderer->get_device()->CreateBuffer(&light_buffer_desc, nullptr, &renderer->m_constant_buffer_light);

    assert(SUCCEEDED(hr));

    renderer->create_depth_stencil();
    renderer->create_rasterizer_state();

    auto const viewport = create_viewport(screen_width, screen_height);
    renderer->g_pd3dDeviceContext->RSSetViewports(1, &viewport);

    return renderer;
}

RendererDX11::RendererDX11(AK::Badge<RendererDX11>)
{
}

void RendererDX11::on_window_resize(GLFWwindow* window, i32 const width, i32 const height)
{
    auto const renderer = get_instance_dx11();

    renderer->cleanup_depth_stencil();
    renderer->cleanup_render_target();
    renderer->cleanup_render_texture();

    renderer->screen_height = height;
    renderer->screen_width = width;

    HRESULT const hr = renderer->g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    assert(SUCCEEDED(hr));

    if (width == 0 || height == 0)
    {
        return;
    }

    renderer->create_render_texture();
    renderer->create_render_target();
    renderer->create_depth_stencil();

    auto const viewport = create_viewport(width, height);
    renderer->g_pd3dDeviceContext->RSSetViewports(1, &viewport);
}

void RendererDX11::begin_frame() const
{
    // This function could be called like an event, instead is called every frame (could slow down, but I do not think so).
    get_instance_dx11()->create_rasterizer_state();
    Renderer::begin_frame();

    if (m_render_to_texture)
    {
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_textureRenderTargetView, m_depth_stencil_view);
    }
    else
    {
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, m_depth_stencil_view);
    }

    float const clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    g_pd3dDeviceContext->ClearRenderTargetView(g_textureRenderTargetView, clear_color_with_alpha);
    g_pd3dDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RendererDX11::end_frame() const
{
    Renderer::end_frame();

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, m_depth_stencil_view);
}

void RendererDX11::present() const
{
    Renderer::present();

    g_pSwapChain->Present(vsync_enabled, 0);
}

ID3D11Device* RendererDX11::get_device() const
{
    return g_pd3dDevice;
}

ID3D11DeviceContext* RendererDX11::get_device_context() const
{
    return g_pd3dDeviceContext;
}

ID3D11ShaderResourceView* RendererDX11::get_render_texture_view() const
{
    return m_render_target_texture_view;
}

void RendererDX11::update_shader(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view,
                                 glm::mat4 const& projection_view_no_translation) const
{
}

void RendererDX11::update_material(std::shared_ptr<Material> const& material) const
{
}

void RendererDX11::update_object(std::shared_ptr<Drawable> const& drawable, std::shared_ptr<Material> const& material,
                                 glm::mat4 const& projection_view) const
{
    ConstantBufferPerObject data = {};
    glm::mat4 const model = drawable->entity->transform->get_model_matrix();
    data.projection_view_model = projection_view * model;
    data.model = drawable->entity->transform->get_model_matrix();
    data.projection = Camera::get_main_camera()->get_projection();

    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    HRESULT const hr = get_device_context()->Map(m_constant_buffer_per_object, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

    assert(SUCCEEDED(hr));

    CopyMemory(mapped_resource.pData, &data, sizeof(ConstantBufferPerObject));

    get_device_context()->Unmap(m_constant_buffer_per_object, 0);
    get_device_context()->VSSetConstantBuffers(0, 1, &m_constant_buffer_per_object);
    
    set_light_buffer(drawable);
}

void RendererDX11::initialize_global_renderer_settings()
{
    ID3D11BlendState* blend_state = nullptr;
    D3D11_BLEND_DESC blend_desc = {};

    blend_desc.RenderTarget[0].BlendEnable = true;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT const hr = get_device()->CreateBlendState(&blend_desc, &blend_state);

    assert(SUCCEEDED(hr));

    std::array constexpr blend_factor = { 0.0f, 0.0f, 0.0f, 0.0f };
    get_device_context()->OMSetBlendState(blend_state, blend_factor.data(), 0xffffffff);
}

void RendererDX11::initialize_buffers(size_t const max_size)
{
}

void RendererDX11::perform_frustum_culling(std::shared_ptr<Material> const& material) const
{
}

D3D11_VIEWPORT RendererDX11::create_viewport(i32 const width, i32 const height)
{
    return {
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height),
        0.0f,
        1.0f
    };
}

void RendererDX11::set_light_buffer(std::shared_ptr<Drawable> const& drawable) const
{
    ConstantBufferLight light_data = {};

    if (m_directional_light != nullptr)
    {
        light_data.directional_light.direction = m_directional_light->entity->transform->get_forward();
        light_data.directional_light.ambient = m_directional_light->ambient;
        light_data.directional_light.diffuse = m_directional_light->diffuse;
        light_data.directional_light.specular = m_directional_light->specular;
    }

    i32 num_point_lights;
    if (m_point_lights.size() < MAX_POINT_LIGHTS)
    {
        num_point_lights = m_point_lights.size();
    }
    else
    {
        num_point_lights = MAX_POINT_LIGHTS;

        // Sort lights by distance
        std::ranges::sort(m_point_lights, [drawable](std::shared_ptr<PointLight> const& a, std::shared_ptr<PointLight> const& b) {
            return glm::length(drawable->entity->transform->get_position() - a->entity->transform->get_position()) <
                   glm::length(drawable->entity->transform->get_position() - b->entity->transform->get_position());
        });
    }

    for (i32 i = 0; i < num_point_lights; i++)
    {
        light_data.point_lights[i].position = m_point_lights[i]->entity->transform->get_position();
        light_data.point_lights[i].ambient = m_point_lights[i]->ambient;
        light_data.point_lights[i].diffuse = m_point_lights[i]->diffuse;
        light_data.point_lights[i].specular = m_point_lights[i]->specular;
        light_data.point_lights[i].constant = m_point_lights[i]->constant;
        light_data.point_lights[i].linear = m_point_lights[i]->linear;
        light_data.point_lights[i].quadratic = m_point_lights[i]->quadratic;
    }

    i32 num_spot_lights;
    if (m_spot_lights.size() < MAX_SPOT_LIGHTS)
    {
        num_spot_lights = m_spot_lights.size();
    }
    else
    {
        num_spot_lights = MAX_SPOT_LIGHTS;

        // Sort lights by distance
        std::ranges::sort(m_spot_lights, [drawable](std::shared_ptr<SpotLight> const& a, std::shared_ptr<SpotLight> const& b) {
            return glm::length(drawable->entity->transform->get_position() - a->entity->transform->get_position()) <
                   glm::length(drawable->entity->transform->get_position() - b->entity->transform->get_position());
        });
    }

    for (i32 i = 0; i < num_spot_lights; i++)
    {
        light_data.spot_lights[i].position = m_spot_lights[i]->entity->transform->get_position();
        light_data.spot_lights[i].direction = m_spot_lights[i]->entity->transform->get_forward();
        light_data.spot_lights[i].cut_off = m_spot_lights[i]->cut_off;
        light_data.spot_lights[i].outer_cut_off = m_spot_lights[i]->outer_cut_off;

        light_data.spot_lights[i].constant = m_spot_lights[i]->constant;
        light_data.spot_lights[i].linear = m_spot_lights[i]->linear;
        light_data.spot_lights[i].quadratic = m_spot_lights[i]->quadratic;

        light_data.spot_lights[i].ambient = m_spot_lights[i]->ambient;
        light_data.spot_lights[i].diffuse = m_spot_lights[i]->diffuse;
        light_data.spot_lights[i].specular = m_spot_lights[i]->specular;
    }

    light_data.camera_pos = Camera::get_main_camera()->entity->transform->get_position();
    light_data.number_of_point_lights = num_point_lights;
    light_data.number_of_spot_lights = num_spot_lights;

    D3D11_MAPPED_SUBRESOURCE mapped_light_buffer_resource = {};
    HRESULT const hr = get_device_context()->Map(m_constant_buffer_light, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_light_buffer_resource);

    assert(SUCCEEDED(hr));

    CopyMemory(mapped_light_buffer_resource.pData, &light_data, sizeof(ConstantBufferLight));
    get_device_context()->Unmap(m_constant_buffer_light, 0);
    get_device_context()->PSSetConstantBuffers(0, 1, &m_constant_buffer_light);
}

bool RendererDX11::create_device_d3d(HWND const hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    u32 create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    D3D_FEATURE_LEVEL constexpr feature_level_array[2] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT const hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        create_device_flags,
        feature_level_array,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        &feature_level,
        &g_pd3dDeviceContext
    );

    if (FAILED(hr))
    {
        return false;
    }

    create_render_texture();
    create_render_target();

    return true;
}

void RendererDX11::cleanup_device_d3d()
{
    cleanup_depth_stencil();
    cleanup_render_target();
    cleanup_render_texture();

    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }

    if (g_pd3dDeviceContext)
    {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }

    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void RendererDX11::create_render_target()
{
    ID3D11Texture2D* p_back_buffer;
    HRESULT hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));

    assert(SUCCEEDED(hr));

    hr = g_pd3dDevice->CreateRenderTargetView(p_back_buffer, nullptr, &g_mainRenderTargetView);

    assert(SUCCEEDED(hr));

    p_back_buffer->Release();
}

void RendererDX11::create_rasterizer_state()
{
    D3D11_RASTERIZER_DESC wfdesc = {};
    if (wireframe_mode_active)
    {
        wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    }
    else 
    {
        wfdesc.FillMode = D3D11_FILL_SOLID;
    }
    wfdesc.CullMode = D3D11_CULL_NONE;

    HRESULT const hr = g_pd3dDevice->CreateRasterizerState(&wfdesc, &g_rasterizer_state);

    assert(SUCCEEDED(hr));

    g_pd3dDeviceContext->RSSetState(g_rasterizer_state);
}

void RendererDX11::create_depth_stencil()
{
    D3D11_TEXTURE2D_DESC depth_stencil_desc = {};
    depth_stencil_desc.Width = screen_width;
    depth_stencil_desc.Height = screen_height;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.ArraySize = 1;
    depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.SampleDesc.Quality = 0;
    depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_stencil_desc.CPUAccessFlags = 0;
    depth_stencil_desc.MiscFlags = 0;

    HRESULT hr = g_pd3dDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &m_depth_stencil_buffer);

    assert(SUCCEEDED(hr));

    hr = g_pd3dDevice->CreateDepthStencilView(m_depth_stencil_buffer, nullptr, &m_depth_stencil_view);

    assert(SUCCEEDED(hr));

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, m_depth_stencil_view);
}

void RendererDX11::cleanup_depth_stencil()
{
    if (m_depth_stencil_buffer != nullptr)
    {
        m_depth_stencil_buffer->Release();
        m_depth_stencil_buffer = nullptr;
    }

    if (m_depth_stencil_view != nullptr)
    {
        m_depth_stencil_view->Release();
        m_depth_stencil_view = nullptr;
    }
}

void RendererDX11::cleanup_render_target()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

void RendererDX11::create_render_texture()
{
    if (m_render_target_texture)
        m_render_target_texture->Release();

    D3D11_TEXTURE2D_DESC render_texture_desc = {};
    render_texture_desc.Width = screen_width;
    render_texture_desc.Height = screen_height;
    render_texture_desc.MipLevels = 1;
    render_texture_desc.ArraySize = 1;
    render_texture_desc.Format = m_render_target_format;
    render_texture_desc.SampleDesc.Count = 1;
    render_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    render_texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    render_texture_desc.CPUAccessFlags = 0;
    render_texture_desc.MiscFlags = 0;

    HRESULT hr = get_device()->CreateTexture2D(&render_texture_desc, nullptr, &m_render_target_texture);

    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
    shader_resource_view_desc.Format = render_texture_desc.Format;
    shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shader_resource_view_desc.Texture2D.MipLevels = 1;
    shader_resource_view_desc.Texture2D.MostDetailedMip = 0;

    hr = get_device()->CreateShaderResourceView(m_render_target_texture, &shader_resource_view_desc, &m_render_target_texture_view);

    assert(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    render_target_view_desc.Format = m_render_target_format;
    render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    render_target_view_desc.Texture2D.MipSlice = 0;

    hr = g_pd3dDevice->CreateRenderTargetView(m_render_target_texture, &render_target_view_desc, &g_textureRenderTargetView);

    assert(SUCCEEDED(hr));
}

void RendererDX11::cleanup_render_texture()
{
    if (m_render_target_texture_view)
    {
        m_render_target_texture_view->Release();
        m_render_target_texture_view = nullptr;
    }

    if (g_textureRenderTargetView)
    {
        g_textureRenderTargetView->Release();
        g_textureRenderTargetView = nullptr;
    }
}
