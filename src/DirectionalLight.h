#pragma once

#include <glm/glm.hpp>

#include "AK/Badge.h"
#include "Light.h"

class DirectionalLight final : public Light
{
public:
    static std::shared_ptr<DirectionalLight> create();

    virtual void on_destroyed() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    explicit DirectionalLight(AK::Badge<DirectionalLight> badge) : Light()
    {
    }

    glm::mat4 get_projection_view_matrix();

    void set_render_target_for_shadow_mapping() const;

protected:
    virtual void set_up_shadow_mapping() override;

private:
    glm::mat4 m_projection_view_matrix = {};
    ID3D11DepthStencilView* m_shadow_depth_stencil_view = nullptr;
};
