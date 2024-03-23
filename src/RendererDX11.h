#pragma once

#include "Engine.h"
#include "Renderer.h"

class RendererDX11 final : public Renderer
{
public:
    static std::shared_ptr<RendererDX11> create();
    explicit RendererDX11(AK::Badge<RendererDX11>);

    ~RendererDX11() override = default;

    static std::shared_ptr<RendererDX11> get_instance_dx11()
    {
        return m_instance_dx11;
    }

    // Recreating RenderTargetView every time the window is resized makes ImGui always keep its size
    static void on_window_resize(GLFWwindow* window, int width, int height)
    {
        const auto dx11_renderer = get_instance_dx11();

        dx11_renderer->g_mainRenderTargetView->Release();
        dx11_renderer->g_pSwapChain->ResizeBuffers(0, LOWORD(width), HIWORD(height), DXGI_FORMAT_UNKNOWN, 0);
        ID3D11Texture2D* pBackBuffer;
        dx11_renderer->g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        dx11_renderer->get_device()->CreateRenderTargetView(pBackBuffer, NULL, &dx11_renderer->g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    virtual void begin_frame() const override;
    virtual void end_frame() const override;
    virtual void present() const override;

    [[nodiscard]] ID3D11Device* get_device() const;
    [[nodiscard]] ID3D11DeviceContext* get_device_context() const;

protected:
    virtual void update_shader(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const override;
    virtual void update_material(std::shared_ptr<Material> const& material) const override;
    virtual void update_object(std::shared_ptr<Drawable> const& drawable, std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const override;

private:
    virtual void initialize_global_renderer_settings() override;
    virtual void initialize_buffers(size_t const max_size) override;
    virtual void perform_frustum_culling(std::shared_ptr<Material> const& material) const override;

    [[nodiscard]] bool create_device_d3d(HWND const hwnd);
    void cleanup_device_d3d();
    void create_render_target();
    void cleanup_render_target();

    static void set_instance_dx11(std::shared_ptr<RendererDX11> const& renderer)
    {
        m_instance_dx11 = renderer;
    }

    inline static std::shared_ptr<RendererDX11> m_instance_dx11;

    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    ID3D11Buffer* m_constant_buffer_per_object = nullptr;
};
