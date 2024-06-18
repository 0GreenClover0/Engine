#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Collider2D.h"
#include "Component.h"

class IceBound final : public Component
{
public:
    static std::shared_ptr<IceBound> create();

    explicit IceBound(AK::Badge<IceBound>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

private:
    ColliderType2D m_type = ColliderType2D::Rectangle;
    u32 m_size = 1;
};
