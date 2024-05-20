#pragma once

#include "Model.h"

NON_SERIALIZED
class Ellipse final : public Model
{
public:
    static std::shared_ptr<Ellipse> create();
    static std::shared_ptr<Ellipse> create(float const center_x, float const center_z, float const radius_x, float const radius_z,
                                           i32 const segment_count, std::shared_ptr<Material> const& material);

    explicit Ellipse(AK::Badge<Ellipse>);
    Ellipse(AK::Badge<Ellipse>, float center_x, float center_z, float radius_x, float radius_z, i32 segment_count,
            std::shared_ptr<Material> const& material);

    float center_x = 0.0f;
    float center_z = 0.0f;
    float radius_x = 5.0f;
    float radius_z = 5.0f;
    i32 segment_count = 40;

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_ellipse() const;
};
