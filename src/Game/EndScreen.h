#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Button.h"
#include "Component.h"
#include "Popup.h"

class EndScreen final : public Popup
{
public:
    static std::shared_ptr<EndScreen> create();

    explicit EndScreen(AK::Badge<EndScreen>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_enabled() override;
    virtual void on_disabled() override;
#if EDITOR
    virtual void draw_editor() override;
#endif
    void update_background();
    void update_star(u32 const star_number);

    void next_level();
    void restart();
    void menu();

    virtual void hide() override;

    bool is_failed = false;
    u32 number_of_stars = 1;

    std::vector<std::weak_ptr<Entity>> stars = {};

    glm::vec2 star_scale = {};

    std::weak_ptr<Button> next_level_button = {};
    std::weak_ptr<Button> restart_button = {};
    std::weak_ptr<Button> menu_button = {};

private:
    u32 m_shown_stars = 0;
    bool m_is_animation_end = false;

    std::string m_failed_background_path = "./res/textures/UI/end_screen_try_again.png";
    std::string m_win_background_path = "./res/textures/UI/end_screen_level_completed.png";
    std::string m_win_background_tutorial_path = "./res/textures/UI/end_screen_level_completed_tutorial.png";
};
