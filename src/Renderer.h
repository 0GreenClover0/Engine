#pragma once

#include <imgui.h>
#include <set>

#include <glm/mat4x4.hpp>

#include "DirectionalLight.h"
#include "Drawable.h"
#include "Light.h"
#include "Mesh.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Texture.h"
#include "Vertex.h"

class Renderer
{
public:
    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;

    void initialize();

    void register_shader(std::shared_ptr<Shader> const& shader);
    void unregister_shader(std::shared_ptr<Shader> const& shader);

    void register_drawable(std::shared_ptr<Drawable> const& drawable);

    void register_material(std::shared_ptr<Material> const& material);
    void unregister_material(std::shared_ptr<Material> const& material);

    void register_light(std::shared_ptr<Light> const& light);
    void unregister_light(std::shared_ptr<Light> const& light);

    void virtual begin_frame() const;
    void render() const;
    void virtual end_frame() const;
    void virtual present() const;

    static std::shared_ptr<Renderer> get_instance()
    {
        return m_instance;
    }

    enum class RendererApi
    {
        OpenGL,
        DirectX11,
    };

    inline static RendererApi renderer_api = RendererApi::OpenGL;

    bool wireframe_mode_active = false;

    inline static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    inline static i32 screen_width = 1280;
    inline static i32 screen_height = 720;

protected:
    Renderer() = default;
    virtual ~Renderer() = default;

    static void set_instance(std::shared_ptr<Renderer> const& renderer)
    {
        m_instance = renderer;
    }

    void virtual initialize_global_renderer_settings() = 0;
    void virtual initialize_buffers(size_t const max_size) = 0;
    void virtual perform_frustum_culling(std::shared_ptr<Material> const& material) const = 0;

    inline static std::shared_ptr<Renderer> m_instance;

private:
    void draw(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const;
    void draw_instanced(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const;

    void set_shader_uniforms(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const;

    inline static std::vector<std::shared_ptr<PointLight>> m_point_lights = {};
    inline static std::vector<std::shared_ptr<SpotLight>> m_spot_lights = {};
    inline static std::shared_ptr<DirectionalLight> m_directional_light = {};

    struct MaterialWithOrder
    {
        i32 render_order;
        std::shared_ptr<Material> material;

        bool operator<(MaterialWithOrder const& b) const
        {
            return render_order < b.render_order;
        }
    };

    std::vector<std::shared_ptr<Light>> m_lights = {};
    std::vector<std::shared_ptr<Shader>> m_shaders = {};
    std::vector<std::shared_ptr<Material>> m_instanced_materials = {};

    std::multiset<MaterialWithOrder> m_custom_render_order_materials = {};

    // TODO: Retrieve this information from the shader
    // NOTE: This has to be the same value as the variable in a shader to work in all cases.
    i32 m_max_point_lights = 4;
    i32 m_max_spot_lights = 4;
};
