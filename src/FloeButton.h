#pragma once
#include "AK/Badge.h"
#include "Component.h"

enum class FloeButtonType
{
    Undefined,
    Start,
    Credits,
    Exit
};

class FloeButton : public Component
{
public:
    static std::shared_ptr<FloeButton> create();
    explicit FloeButton(AK::Badge<FloeButton>);

    virtual void awake() override;
    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    FloeButtonType floe_button_type = FloeButtonType::Undefined;

    bool m_hovered_start = false;
    bool m_hovered_credits = false;
    bool m_hovered_exit = false;
};
