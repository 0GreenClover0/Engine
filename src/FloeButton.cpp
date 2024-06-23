#include "FloeButton.h"

#include "ExampleDynamicText.h"

std::shared_ptr<FloeButton> FloeButton::create()
{
    auto floe = std::make_shared<FloeButton>(AK::Badge<FloeButton> {});
    return floe;
}

FloeButton::FloeButton(AK::Badge<FloeButton>)
{
}

#if EDITOR
void FloeButton::draw_editor()
{
    Component::draw_editor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Dropdown list
    std::array const floe_button_types = {"Undefined", "Start", "Credits", "Exit"};
    i32 current_item_index = static_cast<i32>(floe_button_type);
    if (ImGui::Combo("Floe Button Type", &current_item_index, floe_button_types.data(), floe_button_types.size()))
    {
        floe_button_type = static_cast<FloeButtonType>(current_item_index);
    }
}
#endif
