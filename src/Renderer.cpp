#include "Renderer.h"

#include <format>
#include <iostream>
#include <glad/glad.h>

#include "AK.h"
#include "Camera.h"
#include "Entity.h"

std::shared_ptr<Renderer> Renderer::create()
{
    // We can't use make_shared here because the constructor is private.
    // https://stackoverflow.com/questions/56735974/access-private-constructor-from-public-static-member-function-using-shared-ptr-i
    auto renderer = std::shared_ptr<Renderer>(new Renderer());

    assert(instance == nullptr);

    set_instance(renderer);

    return renderer;
}

void Renderer::initialize()
{
    // TODO: GPU instancing on one material currently supports only the first mesh that was bound to the material.
    int32_t index = 0;
    for (auto const& [material, drawables] : instanced_drawables)
    {
        material->model_matrices.reserve(drawables.size());

        for (auto const& drawable : drawables)
        {
            auto const drawable_locked = drawable.lock();
            material->model_matrices.emplace_back(drawable_locked->entity->transform->get_model_matrix());
        }

        GLuint ssbo;
        glGenBuffers(1, &ssbo);
        material->ssbo = ssbo;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, material->model_matrices.size() * sizeof(glm::mat4), material->model_matrices.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        index++;
    }
}

void Renderer::register_shader(std::shared_ptr<Shader> const& shader)
{
    if (!shaders_map.contains(shader))
    {
        shaders_map.insert(std::make_pair(shader, std::vector<std::weak_ptr<Drawable>> {}));
    }
}

void Renderer::unregister_shader(std::shared_ptr<Shader> const& shader)
{
    shaders_map.erase(shader);
}

void Renderer::register_drawable(std::weak_ptr<Drawable> const& drawable)
{
    auto const drawable_locked = drawable.lock();

    if (drawable_locked->material->is_gpu_instanced)
    {
        drawable_locked->material->drawables.emplace_back(drawable_locked);
    }

    if (drawable_locked->render_order == 0 && !drawable_locked->material->is_gpu_instanced)
    {
        assert(shaders_map.contains(drawable.lock()->material->shader));

        shaders_map[drawable.lock()->material->shader].emplace_back(drawable);
    }
    else if (drawable_locked->render_order != 0)
    {
        custom_render_order_drawables.insert(std::make_pair(drawable_locked->render_order, drawable));
    }
    else if (drawable_locked->material->is_gpu_instanced)
    {
        if (auto const iterator = instanced_drawables.find(drawable_locked->material); iterator != instanced_drawables.end())
        {
            iterator->second.emplace_back(drawable);
        }
        else
        {
            instanced_drawables.insert(std::make_pair(drawable_locked->material, std::vector { drawable }));
        }
    }
}

void Renderer::register_light(std::shared_ptr<Light> const& light)
{
    if (auto const potential_point_light = std::dynamic_pointer_cast<PointLight>(light))
    {
        point_lights.emplace_back(potential_point_light);
    }
    else if (auto const potential_spot_light = std::dynamic_pointer_cast<SpotLight>(light))
    {
        spot_lights.emplace_back(potential_spot_light);
    }
    else if (auto const potential_directional_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        // Don't assert here
        assert(directional_light == nullptr);

        directional_light = potential_directional_light;
    }

    lights.emplace_back(light);
}

void Renderer::unregister_light(std::shared_ptr<Light> const& light)
{
    AK::swap_and_erase(lights, light);
}

void Renderer::render() const
{
    // Premultiply projection and view matrices
    glm::mat4 const projection_view = Camera::get_main_camera()->projection * Camera::get_main_camera()->get_view_matrix();
    glm::mat4 const projection_view_no_translation = Camera::get_main_camera()->projection * glm::mat4(glm::mat3(Camera::get_main_camera()->get_view_matrix()));

    for (auto const& [shader, drawables] : shaders_map)
    {
        shader->use();

        set_shader_uniforms(shader, projection_view, projection_view_no_translation);
        
        for (auto const& drawable : drawables)
        {
            // TODO: Remove null pointers
            auto const drawable_locked = drawable.lock();
            if (drawable_locked == nullptr)
                continue;

            // Could be beneficial to sort drawables per entities as well
            shader->set_mat4("PVM", projection_view * drawable_locked->entity->transform->get_model_matrix());
            shader->set_mat4("model", drawable_locked->entity->transform->get_model_matrix());

            shader->set_vec3("material.color", glm::vec3(drawable_locked->material->color.x, drawable_locked->material->color.y, drawable_locked->material->color.z));
            shader->set_float("material.specular", drawable_locked->material->specular);
            shader->set_float("material.shininess", drawable_locked->material->shininess);

            shader->set_float("radiusMultiplier", drawable_locked->material->radius_multiplier);
            shader->set_int("sector_count", drawable_locked->material->sector_count);
            shader->set_int("stack_count", drawable_locked->material->stack_count);

            drawable_locked->draw();
        }
    }

    for (auto const& [render_order, drawable] : custom_render_order_drawables)
    {
        auto const drawable_locked = drawable.lock();

        if (drawable_locked == nullptr)
            continue;

        auto const shader = drawable_locked->material->shader;
        shader->use();

        set_shader_uniforms(drawable_locked->material->shader, projection_view, projection_view_no_translation);

        shader->set_mat4("PVM", projection_view * drawable_locked->entity->transform->get_model_matrix());
        shader->set_mat4("model", drawable_locked->entity->transform->get_model_matrix());

        shader->set_vec3("material.color", glm::vec3(drawable_locked->material->color.x, drawable_locked->material->color.y, drawable_locked->material->color.z));
        shader->set_float("material.specular", drawable_locked->material->specular);
        shader->set_float("material.shininess", drawable_locked->material->shininess);

        shader->set_float("radiusMultiplier", drawable_locked->material->radius_multiplier);
        shader->set_int("sector_count", drawable_locked->material->sector_count);
        shader->set_int("stack_count", drawable_locked->material->stack_count);

        drawable_locked->draw();
    }

    for (auto const& [material, drawables] : instanced_drawables)
    {
        auto const first_drawable = material->first_drawable;
        auto const shader = first_drawable->material->shader;

        shader->use();

        set_shader_uniforms(shader, projection_view, projection_view_no_translation);

        shader->set_vec3("material.color", glm::vec3(first_drawable->material->color.x, first_drawable->material->color.y, first_drawable->material->color.z));
        shader->set_float("material.specular", first_drawable->material->specular);
        shader->set_float("material.shininess", first_drawable->material->shininess);

        material->model_matrices.clear();
        material->model_matrices.reserve(material->drawables.size());

        for (int32_t i = 0; i < material->drawables.size(); ++i)
        {
            material->model_matrices.emplace_back(drawables[i].lock()->entity->transform->get_model_matrix());
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, material->ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->model_matrices.size() * sizeof(glm::mat4), material->model_matrices.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        first_drawable->draw_instanced(drawables.size());
    }
}

void Renderer::set_shader_uniforms(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    // TODO: Check if shader was already processed and don't perform any operations?
    // TODO: Ultimately we would probably want to cache the uniform location instead of retrieving them by name

    shader->set_vec3("cameraPosition", Camera::get_main_camera()->position);
    shader->set_mat4("PV", projection_view);
    shader->set_mat4("PVnoTranslation", projection_view_no_translation);

    // TODO: Choose only the closest lights

    for (uint32_t i = 0; i < point_lights.size(); ++i)
    {
        std::string light_element = std::format("pointLights[{}].", i);
        shader->set_vec3(light_element + "position", point_lights[i]->entity->transform->get_local_position());

        shader->set_vec3(light_element + "ambient", point_lights[i]->ambient);
        shader->set_vec3(light_element + "diffuse", point_lights[i]->diffuse);
        shader->set_vec3(light_element + "specular", point_lights[i]->specular);

        shader->set_float(light_element + "constant", point_lights[i]->constant);
        shader->set_float(light_element + "linear", point_lights[i]->linear);
        shader->set_float(light_element + "quadratic", point_lights[i]->quadratic);
    }

    shader->set_int("pointLightCount", point_lights.size() > max_point_lights ? max_point_lights : point_lights.size());

    for (uint32_t i = 0; i < spot_lights.size(); ++i)
    {
        std::string light_element = std::format("spotLights[{}].", i);
        shader->set_vec3(light_element + "position", spot_lights[i]->entity->transform->get_local_position());
        shader->set_vec3(light_element + "direction", spot_lights[i]->entity->transform->get_euler_angles());

        shader->set_vec3(light_element + "ambient", spot_lights[i]->ambient);
        shader->set_vec3(light_element + "diffuse", spot_lights[i]->diffuse);
        shader->set_vec3(light_element + "specular", spot_lights[i]->specular);

        shader->set_float(light_element + "cutOff", spot_lights[i]->cut_off);
        shader->set_float(light_element + "outerCutOff", spot_lights[i]->outer_cut_off);

        shader->set_float(light_element + "constant", spot_lights[i]->constant);
        shader->set_float(light_element + "linear", spot_lights[i]->linear);
        shader->set_float(light_element + "quadratic", spot_lights[i]->quadratic);
    }

    shader->set_int("spotLightCount", spot_lights.size() > max_spot_lights ? max_spot_lights : spot_lights.size());

    bool const directional_light_on = directional_light != nullptr;
    if (directional_light_on)
    {
        shader->set_vec3("directionalLight.direction", directional_light->entity->transform->get_euler_angles());

        shader->set_vec3("directionalLight.ambient", directional_light->ambient);
        shader->set_vec3("directionalLight.diffuse", directional_light->diffuse);
        shader->set_vec3("directionalLight.specular", directional_light->specular);
    }

    shader->set_bool("directionalLightOn", directional_light_on);
}
