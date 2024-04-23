#include "Rig.h"

#include <vector>

#include "AK/AnimXForm.h"
#include "AK/aiHelpers.h"
#include "glm/gtx/quaternion.hpp"

void Rig::local_to_model(std::vector<xform>& model_pose, const std::vector<xform>& local_pose)
{
	num_bones = local_pose.size();
	model_pose.resize(num_bones);

	for (int i = 0; i < num_bones; i++)
	{
		const int idParent = parents[i];
		if (idParent >= 0)
		{
			// There might be something wrong here: ORDER of multiplication, parent indices or bones
			// might not be sorted by parent
			model_pose[i] = local_pose[i] * model_pose[idParent];
		}
		else
			model_pose[i] = local_pose[i];
	}

	// Remember to: inv bind pose * pose in model space = skinning matrices
}

// DEPRECATED
// But very important when it comes to the algorithm itself
void Rig::multiply_inverse_bind_pose_by_model_pose(const std::vector<xform>& model_pose)
{
	// skinned_pose.resize(numBones);
	//
	//  for(int i = 0; i < numBones; i++)
	//  {
	//  	xform inverse_bind_pose_x_form, skinned_x_form;
	//  	glm::vec3 scale;
	//  	glm::quat rotation;
	//  	glm::vec3 translation;
	//  	glm::vec3 skew;
	//  	glm::vec4 perspective;
	//  	glm::decompose(inverse_bind_pose[i], scale, rotation, translation, skew, perspective);
	//
	//  	inverse_bind_pose_x_form.position = translation;
	//  	inverse_bind_pose_x_form.rotation = rotation;
	//
	//  	skinned_x_form = inverse_bind_pose_x_form * model_pose[i];
	//
	//  	glm::mat4 skinning_matrix = glm::mat4(1.0f);
	//  	skinning_matrix = glm::translate(skinning_matrix, skinned_x_form.position);
	//  	skinning_matrix = skinning_matrix * glm::toMat4(skinned_x_form.rotation);
	//
	//  	skinned_pose[i] = skinning_matrix;
	//  }
}
