#include "RendererGL.h"

#include <array>
#include <format>

#include "Camera.h"
#include "Entity.h"
#include "MeshGL.h"
#include "ShaderFactory.h"
#include "Skybox.h"
#include "TextureLoaderGL.h"

std::shared_ptr<RendererGL> RendererGL::create()
{
    auto renderer = std::make_shared<RendererGL>(AK::Badge<RendererGL> {});

    assert(m_instance == nullptr);

    set_instance(renderer);

    TextureLoaderGL::create();

    renderer->m_frustum_culling_shader = ShaderFactory::create("./res/shaders/frustum_culling.glsl");

    return renderer;
}

RendererGL::RendererGL(AK::Badge<RendererGL>)
{
}

void RendererGL::begin_frame() const
{
    Renderer::begin_frame();

    glViewport(0, 0, screen_width, screen_height);

    if (wireframe_mode_active)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RendererGL::update_shader(std::shared_ptr<Shader> const& shader, glm::mat4 const& projection_view, glm::mat4 const& projection_view_no_translation) const
{
    // TODO: Ultimately we would probably want to cache the uniform location instead of retrieving them by name

    shader->set_vec3("cameraPosition", Camera::get_main_camera()->get_position());
    shader->set_mat4("PV", projection_view);
    shader->set_mat4("PVnoTranslation", projection_view_no_translation);

    // TODO: Choose only the closest lights

    i32 enabled_light_count = 0;
    for (u32 i = 0; i < m_point_lights.size(); ++i)
    {
        if (!m_point_lights[i]->enabled)
            continue;

        std::string light_element = std::format("pointLights[{}].", enabled_light_count);
        shader->set_vec3(light_element + "position", m_point_lights[i]->entity->transform->get_local_position());

        shader->set_vec3(light_element + "ambient", m_point_lights[i]->ambient);
        shader->set_vec3(light_element + "diffuse", m_point_lights[i]->diffuse);
        shader->set_vec3(light_element + "specular", m_point_lights[i]->specular);

        shader->set_float(light_element + "constant", m_point_lights[i]->constant);
        shader->set_float(light_element + "linear", m_point_lights[i]->linear);
        shader->set_float(light_element + "quadratic", m_point_lights[i]->quadratic);

        enabled_light_count++;
    }

    shader->set_int("pointLightCount", enabled_light_count > m_max_point_lights ? m_max_point_lights : enabled_light_count);

    enabled_light_count = 0;
    for (u32 i = 0; i < m_spot_lights.size(); ++i)
    {
        if (!m_spot_lights[i]->enabled)
            continue;

        std::string light_element = std::format("spotLights[{}].", enabled_light_count);
        shader->set_vec3(light_element + "position", m_spot_lights[i]->entity->transform->get_local_position());
        shader->set_vec3(light_element + "direction", m_spot_lights[i]->entity->transform->get_forward());

        shader->set_vec3(light_element + "ambient", m_spot_lights[i]->ambient);
        shader->set_vec3(light_element + "diffuse", m_spot_lights[i]->diffuse);
        shader->set_vec3(light_element + "specular", m_spot_lights[i]->specular);

        shader->set_float(light_element + "cutOff", m_spot_lights[i]->cut_off);
        shader->set_float(light_element + "outerCutOff", m_spot_lights[i]->outer_cut_off);

        shader->set_float(light_element + "constant", m_spot_lights[i]->constant);
        shader->set_float(light_element + "linear", m_spot_lights[i]->linear);
        shader->set_float(light_element + "quadratic", m_spot_lights[i]->quadratic);

        enabled_light_count++;
    }

    shader->set_int("spotLightCount", enabled_light_count > m_max_spot_lights ? m_max_spot_lights : enabled_light_count);

    bool const directional_light_on = m_directional_light != nullptr && m_directional_light->enabled;
    if (directional_light_on)
    {
        shader->set_vec3("directionalLight.direction", m_directional_light->entity->transform->get_forward());

        shader->set_vec3("directionalLight.ambient", m_directional_light->ambient);
        shader->set_vec3("directionalLight.diffuse", m_directional_light->diffuse);
        shader->set_vec3("directionalLight.specular", m_directional_light->specular);
    }

    shader->set_bool("directionalLightOn", directional_light_on);
}

void RendererGL::update_material(std::shared_ptr<Material> const& material) const
{
    material->shader->set_vec3("material.color", glm::vec3(material->color.x, material->color.y, material->color.z));
    material->shader->set_float("material.specular", material->specular);
    material->shader->set_float("material.shininess", material->shininess);

    material->shader->set_float("radiusMultiplier", material->radius_multiplier);
    material->shader->set_int("sector_count", material->sector_count);
    material->shader->set_int("stack_count", material->stack_count);
}

void RendererGL::update_object(std::shared_ptr<Drawable> const& drawable, std::shared_ptr<Material> const& material, glm::mat4 const& projection_view) const
{
    if (material->needs_view_model)
        material->shader->set_mat4("VM", Camera::get_main_camera()->get_view_matrix() * drawable->entity->transform->get_model_matrix());

    if (material->needs_skybox)
        Skybox::get_instance()->bind();

    material->shader->set_mat4("PVM", projection_view * drawable->entity->transform->get_model_matrix());
    material->shader->set_mat4("model", drawable->entity->transform->get_model_matrix());
}

void RendererGL::initialize_global_renderer_settings()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RendererGL::initialize_buffers(size_t const max_size)
{
    glGenBuffers(1, &m_gpu_instancing_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_gpu_instancing_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_size * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gpu_instancing_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_bounding_boxes_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounding_boxes_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_size * sizeof(BoundingBoxShader), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bounding_boxes_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_visible_instances_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_visible_instances_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_size * sizeof(GLuint), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_visible_instances_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RendererGL::perform_frustum_culling(std::shared_ptr<Material> const& material) const
{
    m_frustum_culling_shader->use();

    // Set frustum planes
    auto const frustum_planes = Camera::get_main_camera()->get_frustum_planes();

    for (u32 i = 0; i < 6; ++i)
    {
        m_frustum_culling_shader->set_vec4(std::format("frustumPlanes[{}]", i), frustum_planes[i]);
    }

    // Send bounding boxes
    // TODO: Batch them with all other existing objects and send only once
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounding_boxes_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->bounding_boxes.size() * sizeof(BoundingBoxShader), material->bounding_boxes.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Run frustum culling
    glDispatchCompute((material->drawables.size() / 1024) + 1, 1, 1);

    // Wait for SSBO write to complete
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Read visible_instances SSBO which has value of 1 when a corresponding object is visible and 0 if it is not visible
    auto* visible_instances = new GLuint[material->drawables.size()];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_visible_instances_ssbo);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->drawables.size() * sizeof(GLuint), visible_instances);

    // TODO: Pass visible instances directly to the shader by a shared SSBO. Might not actually be beneficial?
    for (u32 i = 0; i < material->drawables.size(); ++i)
    {
        if (visible_instances[i] == 1)
        {
            material->model_matrices.emplace_back(material->drawables[i]->entity->transform->get_model_matrix());
        }
    }

    // Free visible_instances memory
    delete[] visible_instances;

    material->shader->use();

    // Pass model matrices of visible instances to the GPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_gpu_instancing_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->model_matrices.size() * sizeof(glm::mat4), material->model_matrices.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
