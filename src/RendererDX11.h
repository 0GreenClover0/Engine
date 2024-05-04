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
    static void on_window_resize(GLFWwindow* window, i32 const width, i32 const height);

    virtual void begin_frame() const override;
    virtual void end_frame() const override;
    virtual void present() const override;

    [[nodiscard]] ID3D11Device* get_device() const;
    [[nodiscard]] ID3D11DeviceContext* get_device_context() const;
    [[nodiscard]] ID3D11ShaderResourceView* get_render_texture_view() const;

    ID3D11RenderTargetView* g_emptyRenderTargetView = nullptr;
    static float constexpr SHADOW_MAP_SIZE = 1024.0f;

protected:
    virtual void update_shader(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const override;
    virtual void update_material(std::shared_ptr<Material> const& material) const override;
    virtual void update_object(std::shared_ptr<Drawable> const& drawable, std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const override;

private:
    virtual void initialize_global_renderer_settings() override;
    virtual void initialize_buffers(size_t const max_size) override;
    virtual void perform_frustum_culling(std::shared_ptr<Material> const& material) const override;

    [[nodiscard]] static D3D11_VIEWPORT create_viewport(i32 const width, i32 const height);
    void set_light_buffer(std::shared_ptr<Drawable> const& drawable) const;

    [[nodiscard]] bool create_device_d3d(HWND const hwnd);
    void cleanup_device_d3d();
    void create_render_target();
    void cleanup_render_target();
    void create_render_texture();
    void cleanup_render_texture();
    void create_rasterizer_state();
    void create_depth_stencil();
    void cleanup_depth_stencil();
    static void set_instance_dx11(std::shared_ptr<RendererDX11> const& renderer)
    {
        m_instance_dx11 = renderer;
    }

    void virtual bind_for_render_frame() const override;
    void set_RS_for_shadow_mapping() const;
    void update_depth_shader(std::shared_ptr<Light> const& light) const;
    virtual void render_shadow_maps() const override;
    inline static std::shared_ptr<RendererDX11> m_instance_dx11;

    D3D11_VIEWPORT m_viewport = {};
    D3D11_VIEWPORT m_shadow_map_viewport = {};

    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    ID3D11RenderTargetView* g_textureRenderTargetView = nullptr;

    ID3D11RasterizerState* g_rasterizer_state = nullptr;
    ID3D11Buffer* m_constant_buffer_light = nullptr;
    ID3D11Buffer* m_constant_buffer_per_object = nullptr;
    ID3D11Buffer* m_constant_buffer_point_shadows = nullptr;
    ID3D11DepthStencilView* m_depth_stencil_view = nullptr;
    ID3D11Texture2D* m_depth_stencil_buffer = nullptr;
    ID3D11Texture2D* m_render_target_texture = nullptr;
    ID3D11ShaderResourceView* m_render_target_texture_view = nullptr;

    // Shadow mapping variables
    ID3D11RasterizerState* g_shadow_rasterizer_state = nullptr;
    ID3D11SamplerState* m_shadow_sampler_state = nullptr;


    inline static DXGI_FORMAT m_render_target_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
};
