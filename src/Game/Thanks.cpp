#include "Thanks.h"

#include "AK/AK.h"
#include "Credits.h"
#include "Entity.h"
#include "FloeButton.h"
#include "Popup.h"

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<Thanks> Thanks::create()
{
    return std::make_shared<Thanks>(AK::Badge<Thanks> {});
}

Thanks::Thanks(AK::Badge<Thanks>)
{
}

void Thanks::awake()
{
    set_can_tick(true);
}

void Thanks::on_enabled()
{
    if (!back_to_menu_button.expired())
    {
        back_to_menu_button.lock()->on_unclicked.attach(&Thanks::hide, shared_from_this());
    }
}

void Thanks::on_disabled()
{
    if (!back_to_menu_button.expired())
    {
        back_to_menu_button.lock()->on_unclicked.detach(shared_from_this());
    }
}

void Thanks::update()
{
}

#if EDITOR
void Thanks::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Back to menu button", back_to_menu_button);
}
#endif

void Thanks::hide()
{
    entity->get_component<Popup>()->hide();
}
