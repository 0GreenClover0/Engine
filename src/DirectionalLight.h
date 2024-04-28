#pragma once

#include <glm/glm.hpp>

#include "Light.h"
#include "AK/Badge.h"

class DirectionalLight final : public Light
{
public:
    static std::shared_ptr<DirectionalLight> create();

    virtual void draw_editor() override;

    explicit DirectionalLight(AK::Badge<DirectionalLight> badge) : Light() { }

    glm::mat4 get_projection_view_matrix();

private:
    // Matrices used for shadow mapping
    glm::mat4 m_projection_view_matrix = {};
    glm::mat4 m_last_model_matrix = {};
};
