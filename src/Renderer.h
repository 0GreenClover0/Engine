#pragma once

#include <set>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

#include "DirectionalLight.h"
#include "Drawable.h"
#include "Light.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "AK/Badge.h"

class Renderer
{
public:
    static std::shared_ptr<Renderer> create();

    explicit Renderer(AK::Badge<Renderer>) {}
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

    void render() const;

    static std::shared_ptr<Renderer> get_instance()
    {
        return instance;
    }

    enum class RendererApi
    {
        OpenGL,
    };

    inline static RendererApi renderer_api;

private:
    static void set_instance(std::shared_ptr<Renderer> const& renderer)
    {
        instance = renderer;
    }

    void draw(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const;
    void draw_instanced(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const;

    void set_shader_uniforms(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const;

    inline static std::shared_ptr<Renderer> instance;

    inline static std::vector<std::shared_ptr<PointLight>> point_lights = {};
    inline static std::vector<std::shared_ptr<SpotLight>> spot_lights = {};
    inline static std::shared_ptr<DirectionalLight> directional_light = {};

    struct MaterialWithOrder
    {
        int32_t render_order;
        std::shared_ptr<Material> material;

        bool operator<(MaterialWithOrder const& b) const
        {
            return render_order < b.render_order;
        }
    };

    std::shared_ptr<Shader> frustum_culling_shader = {};

    std::vector<std::shared_ptr<Light>> lights = {};
    std::vector<std::shared_ptr<Shader>> shaders = {};
    std::vector<std::shared_ptr<Material>> instanced_materials = {};

    std::multiset<MaterialWithOrder> custom_render_order_materials = {};

    // TODO: Retrieve this information from the shader
    // NOTE: This has to be the same value as the variable in a shader to work in all cases.
    int32_t max_point_lights = 4;
    int32_t max_spot_lights = 4;

    GLuint gpu_instancing_ssbo = {};
    GLuint bounding_boxes_ssbo = {};
    GLuint visible_instances_ssbo = {};
};
