#pragma once

#include "Component.h"

class Path final : public Component
{
public:
    static std::shared_ptr<Path> create();

    explicit Path(AK::Badge<Path>);

    virtual void draw_editor() override;

    glm::vec2 get_point_at(float x) const;
    
    std::vector<glm::vec2> points = {};

private:
    float length() const;
};
