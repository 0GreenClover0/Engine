#pragma once

#include "Component.h"

#include "AK/Badge.h"

class Ship;

class Port final : public Component
{
public:
    static std::shared_ptr<Port> create();

    explicit Port(AK::Badge<Port>);

    virtual void on_collision_enter(std::shared_ptr<Collider2D> const &other) override;

private:
    std::vector<std::weak_ptr<Ship>> ships_inside = {};
};
