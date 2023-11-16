#pragma once

#include "Model.h"

class Ellipse final : public Model
{
public:
	Ellipse(float center_x, float center_z, float radius_x, float radius_z, int segment_count, std::shared_ptr<Material> const& material);

private:
	[[nodiscard]] Mesh create_ellipse() const;

	float center_x;
	float center_z;
	float radius_x;
	float radius_z;
	int segment_count;
};