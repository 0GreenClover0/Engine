#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

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
    void update_screen_position() const;

private:
    static float ease_out_back(float x);

    bool m_is_in_screen = false;
    float m_appear_counter = 0.0f;
};
