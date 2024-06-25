#include "NowPromptTrigger.h"

#include "Collider2D.h"
#include "Entity.h"
#include "Game/Ship.h"
#include "Globals.h"
#include "SceneSerializer.h"

std::shared_ptr<NowPromptTrigger> NowPromptTrigger::create()
{
    return std::make_shared<NowPromptTrigger>(AK::Badge<NowPromptTrigger> {});
}

NowPromptTrigger::NowPromptTrigger(AK::Badge<NowPromptTrigger>)
{
}

void NowPromptTrigger::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    Component::on_trigger_enter(other);

    if (other->entity->get_component<Ship>() != nullptr)
    {
        std::shared_ptr<Ship> const ship = other->entity->get_component<Ship>();
        if (ship->type == ShipType::FoodMedium && !m_entered_triger)
        {
            m_story_now_prompt = SceneSerializer::load_prefab("NowPrompt");
            m_story_now_prompt.lock()->transform->set_position(m_now_prompt_pos);
            m_time_of_showing_prompt = 0.0f;
            m_entered_triger = true;
        }
    }
}

void NowPromptTrigger::awake()
{
    set_can_tick(true);
}

void NowPromptTrigger::update()
{
    if (m_entered_triger)
        m_time_of_showing_prompt += static_cast<float>(delta_time);

    if (m_time_of_showing_prompt >= 4.0f)
    {
        m_entered_triger = false;
        if (!m_story_now_prompt.expired())
            m_story_now_prompt.lock()->destroy_immediate();
    }
}
