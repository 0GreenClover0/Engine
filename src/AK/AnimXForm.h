#pragma once
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>

struct xform
{
	glm::vec3 position = { 0,0,0 };
	glm::quat rotation = { 0,0,0,1 };
};