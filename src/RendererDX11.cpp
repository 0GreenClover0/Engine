#include "RendererDX11.h"

#include <iostream>
#include "../build/src/TextureLoaderDX11.h"

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

    // TODO: Texture loader
    TextureLoaderDX11::create();

    return renderer;
}

RendererDX11::RendererDX11(AK::Badge<RendererDX11>)
{
}

void RendererDX11::begin_frame() const
{
    Renderer::begin_frame();
}

void RendererDX11::end_frame() const
{
    Renderer::end_frame();

    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
}

void RendererDX11::present() const
{
    Renderer::present();

    g_pSwapChain->Present(1, 0);
}


ID3D11Device* RendererDX11::get_device() const
{
    return g_pd3dDevice;
}

ID3D11DeviceContext* RendererDX11::get_device_context() const
{
    return g_pd3dDeviceContext;
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
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void RendererDX11::cleanup_render_target()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}
