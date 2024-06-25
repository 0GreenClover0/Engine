#pragma once

#include "AK/Badge.h"
#include "Component.h"

class DebugInputController final : public Component
{
public:
    static std::shared_ptr<DebugInputController> create();
    explicit DebugInputController(AK::Badge<DebugInputController>);

    static std::shared_ptr<DebugInputController> get_instance();

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    float gamma = 1.28f;
    float exposure = 1.22f;

private:
    inline static std::shared_ptr<DebugInputController> m_instance = nullptr;
};
