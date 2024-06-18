#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

#include <glm/vec2.hpp>

class Customer;
class Curve;

class CustomerManager final : public Component
{
public:
    static std::shared_ptr<CustomerManager> create();

    explicit CustomerManager(AK::Badge<CustomerManager>);

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    std::vector<std::weak_ptr<Entity>> destinations_after_feeding = {};
    std::weak_ptr<Curve> destination_curve = {};
    std::string customer_prefab = {};

private:
    glm::vec2 get_destination_from_curve(float const x) const;
    static float get_percentage_of_line(u32 const index);

    std::vector<std::weak_ptr<Customer>> m_customers = {};

    u32 m_registered_food = 0;
};
