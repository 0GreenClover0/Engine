#include "Renderer.h"

#include <array>
#include <format>
#include <glad/glad.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "AK/AK.h"
#include "Camera.h"
#include "Debug.h"
#include "Engine.h"
#include "Entity.h"
#include "ShaderFactory.h"
#include "Skybox.h"

#include <filesystem>
#include <glm/gtx/norm.hpp>

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

    load_fonts();
}

void Renderer::uninitialize()
{
    unload_fonts();
}

void Renderer::register_shader(std::shared_ptr<Shader> const& shader)
{
    m_shaders.emplace_back(shader);
}

void Renderer::unregister_shader(std::shared_ptr<Shader> const& shader)
{
    AK::swap_and_erase(m_shaders, shader);
}

bool Renderer::is_drawable_registered(std::shared_ptr<Drawable> const& drawable) const
{
    return std::ranges::find(drawable->material->drawables.begin(), drawable->material->drawables.end(), drawable)
        != drawable->material->drawables.end();
}

void Renderer::register_drawable(std::shared_ptr<Drawable> const& drawable)
{
    drawable->material->drawables.emplace_back(drawable);
}

void Renderer::unregister_drawable(std::shared_ptr<Drawable> const& drawable)
{
    AK::swap_and_erase(drawable->material->drawables, drawable);

    if (drawable->material->drawables.size() == 0)
    {
        unregister_material(drawable->material);
    }
}

void Renderer::register_material(std::shared_ptr<Material> const& material)
{
    if (material->is_gpu_instanced)
    {
        m_instanced_materials.emplace_back(material);
    }

    if (material->has_custom_render_order())
    {
        if (material->get_render_order() <= aa_render_order)
        {
            m_custom_render_order_materials_before_aa.insert({material->get_render_order(), material});
        }
        else
        {
            m_custom_render_order_materials_after_aa.insert({material->get_render_order(), material});
        }
    }

    if (material->is_transparent)
    {
        m_transparent_materials.emplace_back(material);
    }
}

void Renderer::unregister_material(std::shared_ptr<Material> const& material)
{
    if (material->is_gpu_instanced)
    {
        AK::swap_and_erase(m_instanced_materials, material);
    }

    // FIXME: Not sure if find works
    if (material->has_custom_render_order())
    {
        if (material->get_render_order() <= aa_render_order)
        {
            m_custom_render_order_materials_before_aa.erase(
                m_custom_render_order_materials_before_aa.find({material->get_render_order(), material}));
        }
        else
        {
            m_custom_render_order_materials_after_aa.erase(
                m_custom_render_order_materials_after_aa.find({material->get_render_order(), material}));
        }
    }

    if (material->is_transparent)
    {
        AK::swap_and_erase(m_transparent_materials, material);
    }
}

bool Renderer::is_light_registered(std::shared_ptr<Light> const& light) const
{
    if (auto const potential_point_light = std::dynamic_pointer_cast<PointLight>(light))
    {
        return std::ranges::find(m_point_lights.begin(), m_point_lights.end(), potential_point_light) != m_point_lights.end();
    }

    if (auto const potential_spot_light = std::dynamic_pointer_cast<SpotLight>(light))
    {
        return std::ranges::find(m_spot_lights.begin(), m_spot_lights.end(), potential_spot_light) != m_spot_lights.end();
    }

    if (auto const potential_directional_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        return m_directional_light == potential_directional_light;
    }

    std::unreachable();
}

void Renderer::register_light(std::shared_ptr<Light> const& light)
{
    if (auto const potential_point_light = std::dynamic_pointer_cast<PointLight>(light))
    {
        if (m_point_lights.size() >= MAX_POINT_LIGHTS)
        {
            Debug::log("You've reached the limit of point lights!", DebugType::Error);
        }
        else
        {
            m_point_lights.emplace_back(potential_point_light);
        }
    }
    else if (auto const potential_spot_light = std::dynamic_pointer_cast<SpotLight>(light))
    {
        if (m_spot_lights.size() >= MAX_SPOT_LIGHTS)
        {
            Debug::log("You've reached the limit of spot lights!", DebugType::Error);
        }
        else
        {
            m_spot_lights.emplace_back(potential_spot_light);
        }
    }
    else if (auto const potential_directional_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        if (m_directional_light != nullptr)
        {
            Debug::log("You've just added a second directional light to the scene. You need to remove this one, remove the original, and "
                       "then add this one back.",
                       DebugType::Error);
        }

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

void Renderer::register_camera(std::shared_ptr<Camera> const& camera)
{
    m_cameras.emplace_back(camera);
}

void Renderer::unregister_camera(std::shared_ptr<Camera> const& camera)
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

    // Premultiply projection and view matrices
    glm::mat4 const projection_view = Camera::get_main_camera()->get_projection() * Camera::get_main_camera()->get_view_matrix();
    glm::mat4 const projection_view_no_translation =
        Camera::get_main_camera()->get_projection() * glm::mat4(glm::mat3(Camera::get_main_camera()->get_view_matrix()));

    // Renders to G-Buffer
    render_geometry_pass(projection_view);

    render_ssao();

    // We bind resources that are used in both deferred and forward rendering
    bind_universal_resources();

    // Renders opaque objects
    render_lighting_pass();

    // Renders transparent objects
    render_forward_pass(projection_view, projection_view_no_translation);

    render_custom_render_order_before_aa(projection_view, projection_view_no_translation);

    // Render AA (FXAA)
    render_aa();

    // Render custom render order materials, so in our case UI, as it should not be affected by AA
    render_custom_render_order_after_aa(projection_view, projection_view_no_translation);
}

void Renderer::render_geometry_pass(glm::mat4 const& projection_view) const
{
}

void Renderer::render_ssao() const
{
}

void Renderer::render_aa() const
{
}

void Renderer::render_custom_render_order_before_aa(glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    bool drawn_transparent = false;

    for (auto const& [render_order, material] : m_custom_render_order_materials_before_aa)
    {
        if (render_order == transparent_render_order)
        {
            if (!drawn_transparent)
            {
                draw_transparent(projection_view, projection_view_no_translation);
                drawn_transparent = true;
            }

            if (material->is_transparent)
            {
                continue;
            }
        }

        material->shader->use();

        update_shader(material->shader, projection_view, projection_view_no_translation);

        if (material->is_gpu_instanced)
            draw_instanced(material, projection_view, projection_view_no_translation);
        else
            draw(material, projection_view);
    }
}

void Renderer::render_custom_render_order_after_aa(glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    for (auto const& [render_order, material] : m_custom_render_order_materials_after_aa)
    {
        material->shader->use();

        update_shader(material->shader, projection_view, projection_view_no_translation);

        if (material->is_gpu_instanced)
            draw_instanced(material, projection_view, projection_view_no_translation);
        else
            draw(material, projection_view);
    }
}

void Renderer::bind_universal_resources() const
{
}

void Renderer::render_forward_pass(glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    bind_for_render_frame();

    for (auto const& shader : m_shaders)
    {
        shader->use();

        update_shader(shader, projection_view, projection_view_no_translation);

        for (auto const& material : shader->materials)
        {
            if (!material->needs_forward_rendering)
                continue;

            if (material->has_custom_render_order())
                continue;

            if (material->is_gpu_instanced)
                draw_instanced(material, projection_view, projection_view_no_translation);
            else
                draw(material, projection_view);
        }
    }
}

void Renderer::render_lighting_pass() const
{
}

void Renderer::render_single_shadow_map(glm::mat4 const& projection_view) const
{
    for (auto const& shader : m_shaders)
    {
        for (auto const& material : shader->materials)
        {
            if (!material->casts_shadows)
                continue;

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
    float const time = glfwGetTime();

    for (u32 i = 0; i < m_shaders.size(); i++)
    {
        m_shaders[i]->load_shader();
    }

    Debug::log(std::format("Shader reload time: {}", glfwGetTime() - time));
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

        if (material->is_billboard)
        {
            drawable->entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
        }

        drawable->draw();
    }

    unbind_material(material);
}

void Renderer::draw_instanced(std::shared_ptr<Material> const& material, glm::mat4 const& projection_view,
                              glm::mat4 const& projection_view_no_translation) const
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
            drawable->entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
        }
    }

    // TODO: Adjust bounding boxes on GPU?
    for (u32 i = 0; i < material->drawables.size(); ++i)
    {
        if (material->drawables[i]->entity->transform->needs_bounding_box_adjusting)
        {
            material->drawables[i]->bounds =
                material->first_drawable->get_adjusted_bounding_box(material->drawables[i]->entity->transform->get_model_matrix());
            material->bounding_boxes[i] = BoundingBoxShader(material->drawables[i]->bounds);
            material->drawables[i]->entity->transform->needs_bounding_box_adjusting = false;
        }
    }

    perform_frustum_culling(material);

    shader->use();

    //set_shader_uniforms(shader, projection_view, projection_view_no_translation);

    shader->set_vec3("material.color",
                     glm::vec3(first_drawable->material->color.x, first_drawable->material->color.y, first_drawable->material->color.z));
    shader->set_float("material.specular", first_drawable->material->specular);
    shader->set_float("material.shininess", first_drawable->material->shininess);

    first_drawable->draw_instanced(material->model_matrices.size());
}

void Renderer::draw_transparent(glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    std::vector<std::shared_ptr<Drawable>> transparent_drawables = {};

    for (auto const& material : m_transparent_materials)
    {
        transparent_drawables.insert(transparent_drawables.end(), material->drawables.begin(), material->drawables.end());
    }

    std::shared_ptr<Camera> const camera = Camera::get_main_camera();
    glm::vec3 camera_position = camera->entity->transform->get_position();

    std::ranges::sort(transparent_drawables, [&camera_position](std::shared_ptr<Drawable> const& a, std::shared_ptr<Drawable> const& b) {
        float const distance_a = glm::distance2(camera_position, a->entity->transform->get_position());
        float const distance_b = glm::distance2(camera_position, b->entity->transform->get_position());
        return distance_a >= distance_b; // For back-to-front rendering
    });

    for (auto const& drawable : transparent_drawables)
    {
        drawable->material->shader->use();

        update_shader(drawable->material->shader, projection_view, projection_view_no_translation);

#if _DEBUG
        if (drawable->material->is_gpu_instanced)
        {
            Debug::log("GPU instanced transparent materials are not supported.", DebugType::Error);
            return;
        }
#endif

        update_material(drawable->material);

        update_object(drawable, drawable->material, projection_view);

        if (drawable->material->is_billboard)
        {
            drawable->entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
        }

        drawable->draw();

        unbind_material(drawable->material);
    }
}

void Renderer::load_fonts()
{
    bool changed = false;

    for (auto const& path : std::filesystem::recursive_directory_iterator(m_font_path))
    {
        if (path.is_directory())
            continue;

        std::string const path_str = path.path().string();
        LPCSTR const wide_path = path_str.c_str();
        i32 const loaded_fonts_count = AddFontResource(wide_path);

        std::string family_name = path.path().stem().string();

        std::array<std::string, 2> stripped_words = {" Bold", " Light"};

        Font new_font = {};
        new_font.paths.emplace_back(path_str);

        for (auto const& word : stripped_words)
        {
            if (auto const position = family_name.find(word); position != std::string::npos)
            {
                if (word == " Bold")
                {
                    new_font.bold = true;
                }

                family_name.erase(position, word.size());
            }
        }

        new_font.family_name = family_name;

        bool is_new = true;
        for (auto& font : loaded_fonts)
        {
            if (font.family_name == family_name)
            {
                is_new = false;
                font.paths.emplace_back(path_str);
                break;
            }
        }

        if (is_new)
        {
            loaded_fonts.emplace_back(new_font);
        }

        // This might not return more than 0 if the font was already loaded or if it was loaded unsuccessfully, we can't tell for sure.
        if (loaded_fonts_count > 0)
        {
            changed = true;

#if _DEBUG
            Debug::log("Loaded font: " + path.path().string());
#endif
        }
    }

    if (changed)
    {
        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    }
}

void Renderer::unload_fonts()
{
    for (auto const& font : loaded_fonts)
    {
        for (auto const& path : font.paths)
        {
            std::string const path_str = path;
            LPCSTR const wide_path = path_str.c_str();
            i32 const return_value = RemoveFontResource(wide_path);

            if (return_value == 0)
            {
                // Failed to unload a font
                std::cout << "Failed to unload font: " << path_str;
            }
        }
    }

    PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}
