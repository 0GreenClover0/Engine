#pragma once

#include "AK/Badge.h"
#include "Component.h"
#include "Drawable.h"

class Button : public Drawable
{
public:
    static std::shared_ptr<Button> create();
    explicit Button(AK::Badge<Button>, std::shared_ptr<Material> const& material);

    virtual void initialize() override;
    virtual void draw() const override;

    // Detecting clicks and hovers happens only in game.
    virtual void update() override;

    bool is_hovered() const;
    bool is_pressed() const;

    glm::vec2 top_left_corner = {};
    glm::vec2 top_right_corner = {};
    glm::vec2 bottom_left_corner = {};
    glm::vec2 bottom_right_corner = {};

private:
    void calculate_corners_position();
};
