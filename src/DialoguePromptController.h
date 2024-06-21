#pragma once

#include "DialogueObject.h"
#include "Drawable.h"
#include "Panel.h"
#include "ScreenText.h"

enum class InterpolationMode
{
    Show,
    Hide
};

class DialoguePromptController : public Component
{
public:
    static std::shared_ptr<DialoguePromptController> create();
    explicit DialoguePromptController(AK::Badge<DialoguePromptController>);

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void play_content(u16 const vector_index) const;
    float interp_speed = 4.0f; // Unused but maybe will be used

    std::weak_ptr<Panel> dialogue_panel = {};

    std::weak_ptr<ScreenText> upper_text = {};
    std::weak_ptr<ScreenText> middle_text = {};
    std::weak_ptr<ScreenText> lower_text = {};

    std::vector<DialogueObject> dialogue_objects = {};

private:
    void show_or_hide_panel(InterpolationMode const& show);
    u8 get_empty_lines() const;
    void realign_lines() const;

    bool m_perform_panel_move = false;
    InterpolationMode m_interpolation_mode = InterpolationMode::Show;
    float m_interpolation_value = 0.0f;
};
