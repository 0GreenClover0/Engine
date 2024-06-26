#include "Port.h"

#include "AK/AK.h"
#include "AK/Badge.h"
#include "Collider2D.h"
#include "Cube.h"
#include "Entity.h"
#include "LighthouseKeeper.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"
#include "Ship.h"

std::shared_ptr<Port> Port::create()
{
    return std::make_shared<Port>(AK::Badge<Port> {});
}

Port::Port(AK::Badge<Port>)
{
}

void Port::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    auto const ship = other->entity->get_component<Ship>();
    if (ship != nullptr && !ship->is_in_port() && ship->type != ShipType::Pirates)
    {
        ship->stop();
        m_ships_inside.emplace_back(ship);
    }

    if (auto const keeper = other->entity->get_component<LighthouseKeeper>(); keeper != nullptr)
    {
        keeper->set_is_inside_port(true);
        LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::KeeperEnteredPort);
    }
}

void Port::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{
    if (auto const keeper = other->entity->get_component<LighthouseKeeper>(); keeper != nullptr)
    {
        keeper->set_is_inside_port(false);
    }
}

std::vector<std::weak_ptr<Ship>> const& Port::get_ships_inside() const
{
    return m_ships_inside;
}

bool Port::interact(std::shared_ptr<Entity> const& keeper_entity)
{
    if (m_ships_inside.size() <= 0)
        return false;

    std::shared_ptr<Ship> chosen_ship = nullptr;

    float closest_distance = std::numeric_limits<float>::max();
    glm::vec3 const keeper_position = keeper_entity->transform->get_position();

    for (auto const& ship : m_ships_inside)
    {
        if (ship.expired())
        {
            Debug::log(
                "Trying to interact with an expired ship inside the port. Something went wrong with removing the ship from the vector?",
                DebugType::Error);
            continue;
        }

        auto const ship_locked = ship.lock();

        if (ship_locked->type == ShipType::Pirates)
        {
            continue;
        }

        float const distance = glm::distance(keeper_position, ship_locked->entity->transform->get_position());
        if (distance < closest_distance)
        {
            chosen_ship = ship_locked;
            closest_distance = distance;
        }
    }

    if (chosen_ship == nullptr)
    {
        return false;
    }

    auto const& ship = chosen_ship;
    std::shared_ptr<Entity> fish = nullptr;

    switch (ship->type)
    {
    case ShipType::FoodSmall:
        Player::get_instance()->food += 1;
        fish = SceneSerializer::load_prefab("Fish1");
        fish->transform->set_position(ship->entity->transform->get_position());
        break;

    case ShipType::FoodMedium:
        Player::get_instance()->food += 3;
        fish = SceneSerializer::load_prefab("Fish3");
        fish->transform->set_position(ship->entity->transform->get_position());
        break;

    case ShipType::FoodBig:
        Player::get_instance()->food += 5;
        fish = SceneSerializer::load_prefab("Fish5");
        fish->transform->set_position(ship->entity->transform->get_position());
        break;

    case ShipType::Tool:
        Player::get_instance()->packages += 1;
        break;

    case ShipType::Pirates:
        std::unreachable();

    default:
        break;
    }

    AK::erase(m_ships_inside, ship);
    ship->get_collected_by_keeper();

    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::PackageCollected);
    return true;
}

float Port::get_interactable_distance() const
{
    return m_interactable_distance;
}

#if EDITOR
void Port::draw_editor()
{
    Component::draw_editor();

    if (entity->get_component<Collider2D>() == nullptr)
    {
        if (ImGui::Button("Add collider"))
        {
            entity->add_component(Collider2D::create(glm::vec2(1.0f, 1.0f), false));
            entity->get_component<Collider2D>()->is_trigger = true;
            entity->get_component<Collider2D>()->set_collider_type(ColliderType2D::Rectangle);
        }

        return;
    }

    if (lights.size() < 4)
    {
        if (ImGui::Button("Add lights"))
        {
            for (auto const& light : lights)
            {
                light.lock()->destroy_immediate();
            }

            lights.clear();

            // I guess reading from file 4 times is a no-no, but it happens only in editor.
            // I would have to make a copy contructor for Entity if I wanted to avoid this (I think).
            auto const& light_ul = SceneSerializer::load_prefab("Buoy");
            light_ul->name = "LightUL";
            auto const& light_ur = SceneSerializer::load_prefab("Buoy");
            light_ur->name = "LightUR";
            auto const& light_bl = SceneSerializer::load_prefab("Buoy");
            light_bl->name = "LightBL";
            auto const& light_br = SceneSerializer::load_prefab("Buoy");
            light_br->name = "LightBR";

            lights.emplace_back(light_ul);
            lights.emplace_back(light_ur);
            lights.emplace_back(light_bl);
            lights.emplace_back(light_br);

            for (auto const& light : lights)
            {
                light.lock()->add_component(PointLight::create());
                light.lock()->transform->set_parent(entity->transform);
            }

            adjust_lights();
        }
    }

    if (lights.size() != 0)
    {
        if (ImGui::Button("Remove lights"))
        {
            for (auto const& light : lights)
            {
                light.lock()->destroy_immediate();
            }

            lights.clear();
        }

        if (ImGui::Button("Adjust lights"))
        {
            adjust_lights();
        }
    }
}
#endif

void Port::adjust_lights() const
{
    u32 index = 0;

    auto const collider = entity->get_component<Collider2D>();

    glm::vec3 const upper_left = {-collider->width / 2.0f, 0.5f, -collider->height / 2.0f};
    glm::vec3 const upper_right = {collider->width / 2.0f, 0.5f, -collider->height / 2.0f};
    glm::vec3 const bottom_left = {-collider->width / 2.0f, 0.5f, collider->height / 2.0f};
    glm::vec3 const bottom_right = {collider->width / 2.0f, 0.5f, collider->height / 2.0f};

    for (auto const& light : lights)
    {
        auto const& light_comp = light.lock()->get_component<PointLight>();
        light_comp->diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
        light_comp->linear = 10.0f;
        light_comp->quadratic = 10.0f;

        switch (index)
        {
        case 0:
            light.lock()->transform->set_local_position(upper_left);
            break;
        case 1:
            light.lock()->transform->set_local_position(upper_right);
            break;
        case 2:
            light.lock()->transform->set_local_position(bottom_left);
            break;
        case 3:
            light.lock()->transform->set_local_position(bottom_right);
            break;
        default:
            std::unreachable();
        }

        index++;
    }
}
