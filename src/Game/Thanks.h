#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Button.h"
#include "Component.h"

class Thanks final : public Component
{
public:
    static std::shared_ptr<Thanks> create();

    explicit Thanks(AK::Badge<Thanks>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_enabled() override;
    virtual void on_disabled() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void hide();

    std::weak_ptr<Button> back_to_menu_button = {};
};
