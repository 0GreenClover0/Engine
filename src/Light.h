#pragma once

#include <glm/glm.hpp>
#include <d3d11.h>

#include "Component.h"

class Light : public Component
{
public:
    ~Light() override = 0;

    virtual void draw_editor() override;

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void set_up_shadow_mapping() = 0;

    ID3D11ShaderResourceView* const* get_shadow_shader_resource_view_address() const;
    ID3D11ShaderResourceView* get_shadow_shader_resource_view() const;

    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

protected:
    Light() = default;

    glm::mat4 m_last_model_matrix = {};
    ID3D11Texture2D* m_shadow_texture = nullptr;
    ID3D11ShaderResourceView* m_shadow_shader_resource_view = nullptr;
};
