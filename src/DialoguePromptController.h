#pragma once
#include "DialogueObject.h"
#include "Drawable.h"
#include "Panel.h"
#include "ScreenText.h"
#include "Sound.h"

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

    // Those are your friends
    void play_content(u16 vector_index);
    void end_content();

    float interp_speed = 4.0f; // Unused but maybe will be used

    std::weak_ptr<Button> dialogue_panel = {};
    std::weak_ptr<Entity> panel_parent = {};

    std::weak_ptr<ScreenText> upper_text = {};
    std::weak_ptr<ScreenText> middle_text = {};
    std::weak_ptr<ScreenText> lower_text = {};

    std::vector<DialogueObject> dialogue_objects = {};

private:
    void show_or_hide_panel(InterpolationMode const& show);
    u8 get_empty_lines();
    void realign_lines();

    bool m_perform_panel_move = false;
    InterpolationMode m_interpolation_mode = InterpolationMode::Show;
    float m_interpolation_value = 0.0f;
    std::shared_ptr<Sound> m_currently_played_sound = nullptr;
    i32 m_currently_played_content = -1;
};
