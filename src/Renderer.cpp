#include "Renderer.h"

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
    assert(shaders_map.contains(drawable->material->shader));

    shaders_map[drawable.lock()->material->shader].emplace_back(drawable);
}

void Renderer::register_light(std::shared_ptr<Light> const& light)
{
    if (auto const dir_light = std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        assert(directional_light == nullptr);

        directional_light = dir_light;
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

    for (const auto& [shader, drawables] : shaders_map)
    {
        shader->use();

        shader->set_vec3("cameraPosition", Camera::get_main_camera()->position);

        if (directional_light != nullptr)
        {
            shader->set_vec3("light.position", directional_light->entity->transform->get_local_position());
            shader->set_vec3("light.ambient", directional_light->ambient);
            shader->set_vec3("light.diffuse", directional_light->diffuse);
            shader->set_vec3("light.specular", directional_light->specular);
        }
        
        for (auto const& drawable : drawables)
        {
            // TODO: Remove null pointers
            auto const drawable_locked = drawable.lock();
            if (drawable_locked == nullptr)
                return;

            // Could be beneficial to sort drawables per entities as well
            shader->set_mat4("PVM", projection_view * drawable_locked->entity->transform->get_model_matrix());
            shader->set_mat4("model", drawable_locked->entity->transform->get_model_matrix());

            shader->set_vec3("material.color", glm::vec3(drawable_locked->material->color.x, drawable_locked->material->color.y, drawable_locked->material->color.z));
            shader->set_vec3("material.specular", drawable_locked->material->specular);
            shader->set_float("material.shininess", drawable_locked->material->shininess);

            shader->set_float("radiusMultiplier", drawable_locked->material->radius_multiplier);
            shader->set_int("sector_count", drawable_locked->material->sector_count);
            shader->set_int("stack_count", drawable_locked->material->stack_count);

            drawable_locked->draw();
        }
    }
}
