#include "Renderer.h"

#include <array>
#include <format>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include "AK/AK.h"
#include "Camera.h"
#include "Engine.h"
#include "Entity.h"
#include "ShaderFactory.h"
#include "Skybox.h"

void Renderer::initialize()
{
    initialize_global_renderer_settings();

    // TODO: GPU instancing on one material currently supports only the first mesh that was bound to the material.
    size_t max_size = 0;
    for (auto const& material : m_instanced_materials)
    {
        material->model_matrices.reserve(material->drawables.size());
        material->bounding_boxes = std::vector<BoundingBoxShader>(material->drawables.size());

        if (max_size < material->drawables.size())
            max_size = material->drawables.size();
    }

    initialize_buffers(max_size);
}

void Renderer::register_shader(std::shared_ptr<Shader> const& shader)
{
    m_shaders.emplace_back(shader);
}

void Renderer::unregister_shader(std::shared_ptr<Shader> const& shader)
{
    AK::swap_and_erase(m_shaders, shader);
}

void Renderer::register_drawable(std::shared_ptr<Drawable> const& drawable)
{
    drawable->material()->drawables.emplace_back(drawable);
}

void Renderer::unregister_drawable(std::shared_ptr<Drawable> const& drawable)
{
    AK::swap_and_erase(drawable->material()->drawables, drawable);
}

void Renderer::register_material(std::shared_ptr<Material> const& material)
{
    if (material->is_gpu_instanced)
        m_instanced_materials.emplace_back(material);

    if (material->has_custom_render_order())
        m_custom_render_order_materials.insert( { material->get_render_order(), material });
}

void Renderer::unregister_material(std::shared_ptr<Material> const& material)
{
    if (material->is_gpu_instanced)
        AK::swap_and_erase(m_instanced_materials, material);

    // FIXME: Not sure if find works
    if (material->has_custom_render_order())
        m_custom_render_order_materials.erase(m_custom_render_order_materials.find( { material->get_render_order(), material }));
}

void Renderer::register_light(std::shared_ptr<Light> const& light)
{
    if (auto const potential_point_light = std::dynamic_pointer_cast<PointLight>(light))
    {
        m_point_lights.emplace_back(potential_point_light);
    }
    else if (auto const potential_spot_light = std::dynamic_pointer_cast<SpotLight>(light))
    {
        m_spot_lights.emplace_back(potential_spot_light);
    }
    else if (auto const potential_directional_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        // Don't assert here
        assert(m_directional_light == nullptr);

        m_directional_light = potential_directional_light;
    }

    m_lights.emplace_back(light);
}

void Renderer::unregister_light(std::shared_ptr<Light> const& light)
{
    if (auto const potential_point_light = std::dynamic_pointer_cast<PointLight>(light))
    {
        AK::swap_and_erase(m_point_lights, potential_point_light);
    }
    else if (auto const potential_spot_light = std::dynamic_pointer_cast<SpotLight>(light))
    {
        AK::swap_and_erase(m_spot_lights, potential_spot_light);
    }
    else if (auto const potential_directional_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        m_directional_light = nullptr;
    }
}

void Renderer::register_camera(std::shared_ptr<Camera> const &camera)
{
    m_cameras.emplace_back(camera);
}

void Renderer::unregister_camera(std::shared_ptr<Camera> const &camera)
{
    AK::swap_and_erase(m_cameras, camera);
}

void Renderer::choose_main_camera(std::shared_ptr<Camera> const& exclude) const
{
    for (auto const& camera : m_cameras)
    {
        if (camera == exclude)
            continue;

        Camera::set_main_camera(camera);
        break;
    }
}

void Renderer::begin_frame() const
{
    glfwGetFramebufferSize(Engine::window->get_glfw_window(), &screen_width, &screen_height);

    // Update camera
    if (Camera::get_main_camera() != nullptr)
    {
        Camera::get_main_camera()->set_width(static_cast<float>(screen_width));
        Camera::get_main_camera()->set_height(static_cast<float>(screen_height));
    }
}

void Renderer::render() const
{
    if (Camera::get_main_camera() == nullptr)
        return;

    render_shadow_maps();
    bind_for_render_frame();

    // Premultiply projection and view matrices
    glm::mat4 const projection_view = Camera::get_main_camera()->get_projection() * Camera::get_main_camera()->get_view_matrix();
    glm::mat4 const projection_view_no_translation = Camera::get_main_camera()->get_projection() * glm::mat4(glm::mat3(Camera::get_main_camera()->get_view_matrix()));

    for (auto const& shader : m_shaders)
    {
        shader->use();

        update_shader(shader, projection_view, projection_view_no_translation);

        for (auto const& material : shader->materials)
        {
            if (material->has_custom_render_order())
                continue;

            if (material->is_gpu_instanced)
                draw_instanced(material, projection_view, projection_view_no_translation);
            else
                draw(material, projection_view);
        }
    }

    for (auto const& [render_order, material] : m_custom_render_order_materials)
    {
        material->shader->use();

        update_shader(material->shader, projection_view, projection_view_no_translation);

        if (material->is_gpu_instanced)
            draw_instanced(material, projection_view, projection_view_no_translation);
        else
            draw(material, projection_view);
    }
}

void Renderer::render_single_shadow_map(glm::mat4 const& projection_view) const
{
    for (auto const& shader : m_shaders)
    {
        for (auto const& material : shader->materials)
        {
            if (material->is_gpu_instanced)
            {
                // GPU instancing is not implemented in DX11
                std::unreachable();
            }
            else
            {
                draw(material, projection_view);
            }
        }
    }
}

void Renderer::end_frame() const
{
}

void Renderer::present() const
{
}

void Renderer::switch_rendering_to_texture()
{
    m_render_to_texture = !m_render_to_texture;
}

void Renderer::reload_shaders() const
{
    for (u32 i = 0; i < m_shaders.size(); i++)
    {
        m_shaders[i]->load_shader();
    }
}

void Renderer::set_vsync(bool const enabled)
{
    vsync_enabled = enabled;
}

void Renderer::bind_for_render_frame() const
{
}

void Renderer::draw(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const
{
    update_material(material);

    for (auto const& drawable : material->drawables)
    {
        update_object(drawable, material, projection_view);

        drawable->draw();
    }
}

void Renderer::draw_instanced(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    if (material->drawables.empty())
        return;

    auto const first_drawable = material->first_drawable;
    auto const shader = material->shader;

    material->model_matrices.clear();
    material->model_matrices.reserve(material->drawables.size());

    if (material->is_billboard)
    {
        for (auto const& drawable : material->drawables)
        {
            drawable->entity->transform->set_euler_angles(glm::vec3(0.0f, Camera::get_main_camera()->entity->transform->get_euler_angles().y, 0.0f));
        }
    }

    // TODO: Adjust bounding boxes on GPU?
    for (u32 i = 0; i < material->drawables.size(); ++i)
    {
        if (material->drawables[i]->entity->transform->needs_bounding_box_adjusting)
        {
            material->drawables[i]->bounds = material->first_drawable->get_adjusted_bounding_box(material->drawables[i]->entity->transform->get_model_matrix());
            material->bounding_boxes[i] = BoundingBoxShader(material->drawables[i]->bounds);
            material->drawables[i]->entity->transform->needs_bounding_box_adjusting = false;
        }
    }

    perform_frustum_culling(material);

    shader->use();

    //set_shader_uniforms(shader, projection_view, projection_view_no_translation);

    shader->set_vec3("material.color", glm::vec3(first_drawable->material()->color.x, first_drawable->material()->color.y, first_drawable->material()->color.z));
    shader->set_float("material.specular", first_drawable->material()->specular);
    shader->set_float("material.shininess", first_drawable->material()->shininess);

    first_drawable->draw_instanced(material->model_matrices.size());
}
