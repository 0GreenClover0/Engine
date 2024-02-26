#pragma once

#include "Model.h"

class Ellipse final : public Model
{
public:
    Ellipse(float center_x, float center_z, float radius_x, float radius_z, i32 segment_count, std::shared_ptr<Material> const& material);

    virtual std::string get_name() const override;

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_ellipse() const;

    float m_center_x;
    float m_center_z;
    float m_radius_x;
    float m_radius_z;
    i32 m_segment_count;

    friend class SceneSerializer;
};
