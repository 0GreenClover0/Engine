#pragma once

#include <glm/glm.hpp>

#include "AK/Badge.h"
#include "Component.h"

class Curve : public Component
{
public:
    static std::shared_ptr<Curve> create();

    explicit Curve(AK::Badge<Curve>);

#if EDITOR
    virtual void draw_editor() override;
#endif

    std::vector<glm::vec2> points = {};
    glm::vec2 get_point_at(float x) const;
    float get_y_at(float x) const;
    void add_points(std::initializer_list<glm::vec2> new_points);

protected:
    explicit Curve();

private:
    float length() const;
};
