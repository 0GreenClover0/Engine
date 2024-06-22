#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"
#include <Button.h>

class EndScreen final : public Component
{
public:
    static std::shared_ptr<EndScreen> create();

    explicit EndScreen(AK::Badge<EndScreen>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif
    void update_background();
    void update_screen_position() const;
    void update_star(u32 const star_number) const;

    bool is_failed = false;
    u32 number_of_stars = 1;

    std::vector<std::weak_ptr<Entity>> stars = {};

    glm::vec2 star_scale = {};

private:
    static float ease_out_back(float x);
    float ease_out_elastic(float x) const;

    bool m_is_in_screen = false;
    float m_appear_counter = 0.0f;
    u32 m_shown_stars = 0;
    bool m_is_animation_end = false;

    std::string m_failed_background_path = "./res/textures/UI/end_screen_try_again.png";
    std::string m_win_background_path = "./res/textures/UI/end_screen_level_completed.png";
};
