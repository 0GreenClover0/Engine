#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include <vector>
#include "Light.h"
#include "AK/Badge.h"
class PointLight final : public Light
{
public:
    static std::shared_ptr<PointLight> create();
    explicit PointLight(AK::Badge<PointLight>) : Light() { }

    virtual void draw_editor() override;
    virtual std::string get_name() const override;

    void set_render_target_for_shadows(int face) const;
    glm::mat4 get_projection_view_matrix(int face);

    // Default values for an around 50m distance of cover
    float constant = 1.0f; // Should not be changed
    float linear = 0.09f;
    float quadratic = 0.032f;

    
protected:
    virtual void set_up_shadow_mapping() override;

private:
    void update_pv_matrices();

    std::vector<ID3D11DepthStencilView*> m_shadow_depth_stencil_views = {};
    std::vector<glm::mat4> projection_view_matrices = {};
};
