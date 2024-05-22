#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>
#include <imgui.h>

#include "Collider2D.h"
#include "Entity.h"
#include "IceBound.h"
#include "Input.h"

#include "AK/AK.h"
#include "Globals.h"

std::shared_ptr<IceBound> IceBound::create()
{
    return std::make_shared<IceBound>(AK::Badge<IceBound> {});
}

IceBound::IceBound(AK::Badge<IceBound>)
{
}

void IceBound::awake()
{
    set_can_tick(true);
}

void IceBound::update()
{
}
