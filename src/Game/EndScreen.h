#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

class EndScreen final : public Component
{
public:
    static std::shared_ptr<EndScreen> create();
    static std::shared_ptr<EndScreen> create(bool const is_failed);

    explicit EndScreen(AK::Badge<EndScreen>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif
    void update_background();
    void update_screen_position() const;

    bool is_failed = false;

private:
    static float ease_out_back(float x);

    bool m_is_in_screen = false;
    float m_appear_counter = 0.0f;

    std::string m_failed_background_path = "./res/textures/UI/end_screen_try_again.png";
    std::string m_win_background_path = "./res/textures/UI/end_screen_level_completed.png";
};
