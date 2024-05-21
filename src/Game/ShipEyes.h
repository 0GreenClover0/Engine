#pragma once

#include "Component.h"

class ShipEyes final : public Component
{
public:
    static std::shared_ptr<ShipEyes> create();

    explicit ShipEyes(AK::Badge<ShipEyes>);

    virtual void awake() override;
    virtual void update() override;

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;

    NON_SERIALIZED
    bool see_obstacle = false;
};
