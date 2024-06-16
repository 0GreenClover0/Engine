#pragma once

#include <array>
#include <d3d11.h>
#include <glm/glm.hpp>
#include <vector>

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Light.h"

class PointLight final : public Light
{
public:
    static std::shared_ptr<PointLight> create();
    explicit PointLight(AK::Badge<PointLight>) : Light()
    {
    }

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    void set_render_target_for_shadow_mapping(u32 const face_index) const;
    glm::mat4 get_projection_view_matrix(u32 const face_index);

    void set_pulsate(bool const value);
    void set_burn_out(bool const value);
    void set_flash(bool const value);

    // Default values for an around 50m distance of cover
    float constant = 1.0f; // Should not be changed
    float linear = 0.09f;
    float quadratic = 0.032f;

protected:
    virtual void set_up_shadow_mapping() override;

private:
    void update_pv_matrices();

    void pulsate();
    void burn_out();
    void flash();

    static float ease_in_back(float const x);

    bool m_pulsate = false;
    bool m_burn_out = false;
    bool m_flash = false;

    float m_action_timer = 0.0f;

    std::vector<ID3D11DepthStencilView*> m_shadow_depth_stencil_views = {};
    std::array<glm::mat4, 6> m_projection_view_matrices = {};
};
