#include "RendererDX11.h"

#include <iostream>
#include "TextureLoaderDX11.h"
#include "Drawable.h"
#include "Entity.h"

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

    HRESULT const hr = renderer->get_device()->CreateBuffer(&desc, 0, &renderer->m_constant_buffer_per_object);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize constant buffer." << "\n";
    }

    glfwSetWindowSizeCallback(Engine::window->get_glfw_window(), on_window_resize);

    renderer->create_depth_stencil();
    renderer->create_rasterizer_state();

    D3D11_VIEWPORT const viewport = { 0.0f, 0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, 1.0f };
    renderer->g_pd3dDeviceContext->RSSetViewports(1, &viewport);

    return renderer;
}

RendererDX11::RendererDX11(AK::Badge<RendererDX11>)
{
}

void RendererDX11::on_window_resize(GLFWwindow* window, int width, int height)
{
    auto const renderer = get_instance_dx11();
    renderer->get_device_context()->OMSetRenderTargets(0, 0, 0);
    renderer->g_mainRenderTargetView->Release();
    renderer->m_depth_stencil_view->Release();
    renderer->screen_height = height;
    renderer->screen_width = width;

    HRESULT hr;
    hr = renderer->g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    if (FAILED(hr))
    {
        std::cout << "Failed to resize a  buffer!" << std::endl;
    }

    ID3D11Texture2D* pBuffer;
    hr = renderer->g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

    if (FAILED(hr))
    {
        std::cout << "Failed to get a buffer!" << std::endl;
    }

    hr = renderer->get_device()->CreateRenderTargetView(pBuffer, NULL, &renderer->g_mainRenderTargetView);

    if (FAILED(hr))
    {
        std::cout << "Failed to create render target view!" << std::endl;
    }

    // Perform error handling here!
    pBuffer->Release();
    D3D11_TEXTURE2D_DESC depth_stencil_desc;
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
    hr = renderer->g_pd3dDevice->CreateTexture2D(&depth_stencil_desc, NULL, &renderer->m_depth_stencil_buffer);

    if (FAILED(hr))
    {
        std::cout << "Failed to create depth stencil texture." << "\n";
    }

    hr = renderer->g_pd3dDevice->CreateDepthStencilView(renderer->m_depth_stencil_buffer, NULL, &renderer->m_depth_stencil_view);

    if (FAILED(hr))
    {
        std::cout << "Failed to create depth stencil view." << "\n";
    }

    renderer->g_pd3dDeviceContext->OMSetRenderTargets(1, &renderer->g_mainRenderTargetView, renderer->m_depth_stencil_view);

    // Set up the viewport.
    D3D11_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    renderer->g_pd3dDeviceContext->RSSetViewports(1, &vp);
}

void RendererDX11::begin_frame() const
{
    Renderer::begin_frame();
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    g_pd3dDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RendererDX11::end_frame() const
{
    Renderer::end_frame();
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
    ConstantBufferPerObject data;
    data.projection_view = projection_view;
    data.model = drawable->entity->transform->get_model_matrix();
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = get_device_context()->Map(m_constant_buffer_per_object, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    if (FAILED(hr))
    {
        std::cout << "Failed to updated constant buffer." << "\n";
    }

    CopyMemory(mappedResource.pData, &data, sizeof(ConstantBufferPerObject));
    get_device_context()->Unmap(m_constant_buffer_per_object, 0);
    get_device_context()->VSSetConstantBuffers(0, 1, &m_constant_buffer_per_object);
}

void RendererDX11::initialize_global_renderer_settings()
{
}

void RendererDX11::initialize_buffers(size_t const max_size)
{
}

void RendererDX11::perform_frustum_culling(std::shared_ptr<Material> const& material) const
{
}

bool RendererDX11::create_device_d3d(HWND const hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
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

    if (auto const result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &feature_level, &g_pd3dDeviceContext); result != S_OK)
    {
        return false;
    }

    create_render_target();

    return true;
}

void RendererDX11::cleanup_device_d3d()
{
    cleanup_render_target();

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
    ID3D11Texture2D* pBackBuffer;
    HRESULT result = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    assert(SUCCEEDED(result));

    result = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);

    assert(SUCCEEDED(result));

    pBackBuffer->Release();
}

void RendererDX11::create_rasterizer_state()
{
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_SOLID;
    wfdesc.CullMode = D3D11_CULL_NONE;
    HRESULT const hr = g_pd3dDevice->CreateRasterizerState(&wfdesc, &g_rasterizer_state);

    if (FAILED(hr))
    {
        std::cout << "Failed to create rasterizer state." << "\n";
    }

    g_pd3dDeviceContext->RSSetState(g_rasterizer_state);
}

void RendererDX11::create_depth_stencil()
{
    D3D11_TEXTURE2D_DESC depth_stencil_desc;
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

    if (FAILED(hr))
    {
        std::cout << "Failed to create depth stencil texture." << "\n";
    }

    hr = g_pd3dDevice->CreateDepthStencilView(m_depth_stencil_buffer, nullptr, &m_depth_stencil_view);

    if (FAILED(hr))
    {
        std::cout << "Failed to create depth stencil view." << "\n";
    }

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, m_depth_stencil_view);
}

void RendererDX11::cleanup_render_target()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}
