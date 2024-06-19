#pragma once

#include "AK/Types.h"
#include "Component.h"

#include <d3d11.h>

#include <glm/glm.hpp>

class Light : public Component
{
public:
    ~Light() override = 0;

#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void on_destroyed() override;

    virtual void set_up_shadow_mapping() = 0;

    ID3D11ShaderResourceView* const* get_shadow_shader_resource_view_address() const;
    ID3D11ShaderResourceView* get_shadow_shader_resource_view() const;

    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    float m_near_plane = 0.5f;
    float m_far_plane = 25.0f;

    u32 m_blocker_search_num_samples = 16;
    u32 m_pcf_num_samples = 16;
    float m_light_world_size = 0.5f;
    float m_light_frustum_width = 30.0f;

protected:
    Light() = default;

    bool m_planes_changed = true;
    glm::mat4 m_last_model_matrix = {};
    ID3D11Texture2D* m_shadow_texture = nullptr;
    ID3D11ShaderResourceView* m_shadow_shader_resource_view = nullptr;
};
