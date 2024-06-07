#pragma once

#include "Component.h"

class IceBound final : public Component
{
public:
    static std::shared_ptr<IceBound> create();

    explicit IceBound(AK::Badge<IceBound>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

private:
    ColliderType2D m_type = ColliderType2D::Rectangle;
    u32 m_size = 1;
};
