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

#if EDITOR
    virtual void draw_editor() override;
#endif

    FloeButtonType floe_button_type = FloeButtonType::Undefined;
};
