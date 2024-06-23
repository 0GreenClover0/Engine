#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

class Popup : public Component
{
public:
    static std::shared_ptr<Popup> create();

    explicit Popup(AK::Badge<Popup>);

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void update_screen_position();

    void hide();

protected:
    Popup();

    static float ease_out_back(float const x);
    static float ease_out_elastic(float const x);

    bool m_is_in_screen = false;
    float m_appear_counter = 0.0f;

    bool m_is_hiding = false;
};
