#pragma once

#include <vector>

#include "AK/AnimXForm.h"
#include "AK/HashedString.h"
#include "AK/Types.h"

struct Rig
{
	std::vector<hstring> bone_names;
	std::vector<i32> parents;
	std::vector<glm::mat4> inverse_bind_pose;
	glm::mat4 skinned_pose[512] = { glm::mat4(1.0f) };
	u32 num_bones = 0;
	void local_to_model(std::vector<xform>& model_pose, const std::vector<xform>& local_pose);
	void multiply_inverse_bind_pose_by_model_pose(const std::vector<xform>& model_pose);
};
