#include "LighthouseLight.h"

#include "Entity.h"
#include "Input.h"
#include "LevelController.h"
#include "Player.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<LighthouseLight> LighthouseLight::create()
{
    return std::make_shared<LighthouseLight>(AK::Badge<LighthouseLight> {});
}

LighthouseLight::LighthouseLight(AK::Badge<LighthouseLight>)
{
}

void LighthouseLight::on_enabled()
{
    if (!m_sphere.expired())
        m_sphere.lock()->set_enabled(true);
}

void LighthouseLight::on_disabled()
{
    if (!m_sphere.expired())
        m_sphere.lock()->set_enabled(false);
}

void LighthouseLight::awake()
{
    set_can_tick(true);

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/halo.hlsl", "./res/shaders/halo.hlsl");
    auto const standard_material = Material::create(standard_shader);
    standard_material->needs_forward_rendering = true;
    standard_material->casts_shadows = false;
    m_sphere = entity->add_component(Sphere::create(0.25f, 50, 50, "./res/textures/stone.jpg", standard_material));
}

void LighthouseLight::update()
{
    glm::vec2 const position = get_position();

    entity->transform->set_local_position(glm::vec3(position.x, 0.0f, position.y));

    if (spotlight.expired())
        return;

    auto const light_locked = spotlight.lock();

    i32 const lighthouse_level = Player::get_instance()->lighthouse_level;
    light_locked->scattering_factor = 1.0f + static_cast<float>(lighthouse_level) * 2.0f;
    spotlight_beam_width = 0.09f + 0.01f * static_cast<float>(lighthouse_level);

    float const light_beam_length = glm::length(entity->transform->get_position() - light_locked->entity->transform->get_position());
    float const aperture = glm::atan(spotlight_beam_width / light_beam_length);
    light_locked->cut_off = cos(aperture);
    light_locked->outer_cut_off = cos(aperture);
    light_locked->entity->transform->orient_towards(glm::vec3(position.x, 0.0f, position.y));
}

#if EDITOR
void LighthouseLight::draw_editor()
{
    ImGuiEx::draw_ptr("Spotlight", spotlight);
    ImGui::InputFloat("Beam width", &spotlight_beam_width);
}
#endif

void LighthouseLight::set_spot_light(std::shared_ptr<SpotLight> const& spot_light)
{
    spotlight = spot_light;
}

glm::vec2 LighthouseLight::get_position() const
{
    float const y = Input::input->get_mouse_position().y * LevelController::get_instance()->playfield_height
                  + LevelController::get_instance()->playfield_y_shift;
    float const x = Input::input->get_mouse_position().x
                  * (LevelController::get_instance()->playfield_width
                     - (LevelController::get_instance()->playfield_additional_width
                        * ((Input::input->get_mouse_position().y + LevelController::get_instance()->playfield_y_shift) + 1.0f) / 2.0f));

    return glm::vec2(x, y);
}
