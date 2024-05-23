#pragma once

#include "BlurPassContainer.h"
#include "Engine.h"
#include "GBuffer.h"
#include "Renderer.h"
#include "SSAO.h"

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
    [[nodiscard]] ID3D11DepthStencilState* get_depth_stencil_state() const;
    [[nodiscard]] ID3D11DepthStencilView* get_depth_stencil_view() const;
    [[nodiscard]] D3D11_VIEWPORT get_main_view_port() const;

    ID3D11RenderTargetView* g_emptyRenderTargetView = nullptr;
    inline static float constexpr SHADOW_MAP_SIZE = 1024.0f;

    virtual void set_rasterizer_draw_type(RasterizerDrawType const rasterizer_draw_type) override;
    virtual void restore_default_rasterizer_draw_type() override;

protected:
    virtual void update_shader(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view,
                               glm::mat4 const& projection_view_no_translation) const override;
    virtual void update_material(std::shared_ptr<Material> const& material) const override;
    virtual void update_object(std::shared_ptr<Drawable> const& drawable, std::shared_ptr<Material> const& material,
                               glm::mat4 const& projection_view) const override;

    virtual void unbind_material(std::shared_ptr<Material> const& material) const override;

private:
    virtual void initialize_global_renderer_settings() override;
    virtual void initialize_buffers(size_t const max_size) override;
    virtual void perform_frustum_culling(std::shared_ptr<Material> const& material) const override;

    [[nodiscard]] static D3D11_VIEWPORT create_viewport(i32 const width, i32 const height);
    void set_light_buffer() const;
    void set_camera_position_buffer(std::shared_ptr<Drawable> const& drawable) const;

    [[nodiscard]] bool create_device_d3d(HWND const hwnd);
    void cleanup_device_d3d();
    void create_render_target();
    void cleanup_render_target();
    void create_render_texture();
    void cleanup_render_texture();
    void create_rasterizer_state();
    void update_rasterizer_state();
    void create_depth_stencil();
    void cleanup_depth_stencil();
    static void set_instance_dx11(std::shared_ptr<RendererDX11> const& renderer)
    {
        m_instance_dx11 = renderer;
    }

    void virtual bind_for_render_frame() const override;
    void setup_shadow_mapping();
    void set_RS_for_shadow_mapping() const;
    void update_depth_shader(std::shared_ptr<Light> const& light) const;
    virtual void render_shadow_maps() const override;

    virtual void render_lighting_pass() const override;
    virtual void render_geometry_pass(glm::mat4 const& projection_view) const override;
    virtual void render_ssao() const override;

    inline static std::shared_ptr<RendererDX11> m_instance_dx11;

    std::shared_ptr<GBuffer> m_gbuffer = nullptr;
    std::shared_ptr<SSAO> m_ssao = nullptr;
    std::shared_ptr<BlurPassContainer> m_ssao_blur = nullptr;

    D3D11_VIEWPORT m_viewport = {};
    D3D11_VIEWPORT m_shadow_map_viewport = {};

    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    ID3D11RenderTargetView* g_textureRenderTargetView = nullptr;

    ID3D11RasterizerState* g_rasterizer_state_solid = nullptr;
    ID3D11RasterizerState* g_rasterizer_state_wireframe = nullptr;
    ID3D11RasterizerState* g_rasterizer_state = nullptr;

    ID3D11Buffer* m_constant_buffer_light = nullptr;
    ID3D11Buffer* m_constant_buffer_camera_position = nullptr;
    ID3D11Buffer* m_constant_buffer_per_object = nullptr;
    ID3D11Buffer* m_constant_buffer_point_shadows = nullptr;
    ID3D11Buffer* m_constant_buffer_ssao = nullptr;
    ID3D11DepthStencilView* m_depth_stencil_view = nullptr;
    ID3D11Texture2D* m_depth_stencil_buffer = nullptr;
    ID3D11DepthStencilState* m_depth_stencil_state = nullptr;
    ID3D11Texture2D* m_render_target_texture = nullptr;
    ID3D11ShaderResourceView* m_render_target_texture_view = nullptr;

    // Shadow mapping variables
    ID3D11RasterizerState* g_shadow_rasterizer_state = nullptr;
    ID3D11SamplerState* m_shadow_sampler_state = nullptr;
    ID3D11SamplerState* m_default_sampler_state = nullptr;
    ID3D11SamplerState* m_repeat_sampler_state = nullptr;

    ID3D11BlendState* m_deferred_blend_state = nullptr;
    ID3D11BlendState* m_forward_blend_state = nullptr;

    inline static u32 constexpr spot_light_shadow_register_offset = 20;
    inline static u32 constexpr point_light_shadow_register_offset = 40;

    inline static DXGI_FORMAT m_render_target_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
};
