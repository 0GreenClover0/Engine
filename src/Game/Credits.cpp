#include "Credits.h"

#include "AK/AK.h"
#include "Entity.h"
#include "FloeButton.h"
#include "Popup.h"

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<Credits> Credits::create()
{
    return std::make_shared<Credits>(AK::Badge<Credits> {});
}

Credits::Credits(AK::Badge<Credits>)
{
}

void Credits::awake()
{
    set_can_tick(true);
}

void Credits::on_enabled()
{
    if (!back_to_menu_button.expired())
    {
        back_to_menu_button.lock()->on_unclicked.attach(&Credits::hide, shared_from_this());
    }
}

void Credits::on_disabled()
{
    if (!back_to_menu_button.expired())
    {
        back_to_menu_button.lock()->on_unclicked.detach(shared_from_this());
    }
}

void Credits::update()
{
}

#if EDITOR
void Credits::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Back to menu button", back_to_menu_button);
}
#endif

void Credits::hide()
{
    entity->get_component<Popup>()->hide();
    FloeButton::are_credits_open = false;
}
