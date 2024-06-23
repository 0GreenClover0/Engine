#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"
#include "Popup.h"
#include <Button.h>

class EndScreen final : public Popup
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
    void update_star(u32 star_number);

    bool is_failed = false;
    u32 number_of_stars = 1;

    std::vector<std::weak_ptr<Entity>> stars = {};

    glm::vec2 star_scale = {};

private:
    u32 m_shown_stars = 0;
    bool m_is_animation_end = false;

    std::string m_failed_background_path = "./res/textures/UI/end_screen_try_again.png";
    std::string m_win_background_path = "./res/textures/UI/end_screen_level_completed.png";
};
