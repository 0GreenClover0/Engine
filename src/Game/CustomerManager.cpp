#include "CustomerManager.h"

#include "AK/AK.h"
#include "Customer.h"
#include "Entity.h"
#include "LevelController.h"
#include "Player.h"
#include "SceneSerializer.h"

#if EDITOR
#include "imgui_extensions.h"
#include "imgui_stdlib.h"
#endif

std::shared_ptr<CustomerManager> CustomerManager::create()
{
    return std::make_shared<CustomerManager>(AK::Badge<CustomerManager> {});
}

CustomerManager::CustomerManager(AK::Badge<CustomerManager>)
{
}

void CustomerManager::awake()
{
    set_can_tick(true);

    if (destination_curve.expired())
    {
        Debug::log("No destination curve set. Customers won't work properly.", DebugType::Error);
        return;
    }

    for (u32 i = 0; i < LevelController::get_instance()->map_food; i++)
    {
        auto const customer_entity = SceneSerializer::load_prefab(customer_prefab);

        if (customer_entity == nullptr)
        {
            Debug::log("Couldn't load customer prefab.");
            return;
        }

        customer_entity->transform->set_parent(entity->transform);

        glm::vec2 const destination = get_destination_from_curve(get_percentage_of_line(i));

        customer_entity->transform->set_position(AK::convert_2d_to_3d(destination, entity->transform->get_position().y));
        auto const customer = customer_entity->get_component<Customer>();
        customer->desired_height = entity->transform->get_position().y;

        customer->set_destination(AK::convert_2d_to_3d(destination, entity->transform->get_position().y));

        m_customers.emplace_back(customer);
    }
}

void CustomerManager::update()
{
    u32 const customers_to_feed = Player::get_instance()->food - m_registered_food;

    // No food aquired since last frame, nothing to do.
    // FIXME: This could be an event.
    if (customers_to_feed <= 0)
        return;

    for (u32 i = 0; i < customers_to_feed; i++)
    {
        if (m_customers.size() <= 0)
        {
            return;
        }

        m_customers[0].lock()->feed(
            destinations_after_feeding[std::rand() % destinations_after_feeding.size()].lock()->transform->get_position());
        AK::erase(m_customers, m_customers[0].lock());
    }

    for (size_t i = 0; i < m_customers.size(); i++)
    {
        glm::vec2 const destination = get_destination_from_curve(get_percentage_of_line(i));
        m_customers[i].lock()->set_destination(AK::convert_2d_to_3d(destination, entity->transform->get_position().y));
    }

    m_registered_food = Player::get_instance()->food;
}

#if EDITOR
void CustomerManager::draw_editor()
{
    Component::draw_editor();

    for (size_t i = 0; i < destinations_after_feeding.size(); i++)
    {
        ImGuiEx::draw_ptr("Destination After Feeding", destinations_after_feeding[i]);
    }

    if (ImGui::Button("Add element"))
    {
        destinations_after_feeding.emplace_back(std::weak_ptr<Entity> {});
    }

    ImGuiEx::draw_ptr("Destination Curve", destination_curve);
    ImGui::InputText("Customer Prefab", &customer_prefab);
}
#endif

glm::vec2 CustomerManager::get_destination_from_curve(float const x) const
{
    auto const curve_locked = destination_curve.lock();

    glm::vec2 destination = curve_locked->get_point_at(x);
    destination.x += entity->transform->get_position().x;
    destination.y += entity->transform->get_position().z;

    return destination;
}

float CustomerManager::get_percentage_of_line(u32 const index)
{
    return static_cast<float>(index + 1) / static_cast<float>(LevelController::get_instance()->map_food);
}
