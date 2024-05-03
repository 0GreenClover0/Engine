#pragma once

#include <glm/glm.hpp>
#include <d3d11.h>

#include "Component.h"
#include "AK/Badge.h"

class Light : public Component
{
public:
    explicit Light(AK::Badge<Light>) {}

    virtual void draw_editor() override;
    virtual std::string get_name() const override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void set_up_shadow_mapping() = 0;

    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    float m_near_plane = 1.0f;
    float m_far_plane = 25.0f;

    ID3D11ShaderResourceView* m_shadow_shader_resource_view = nullptr;

protected:
    Light() = default;

    ID3D11Texture2D* m_shadow_texture = nullptr;
    glm::mat4 m_last_model_matrix = {};

};
