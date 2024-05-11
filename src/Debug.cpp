#include "Debug.h"

#include "DebugDrawing.h"
#include "Entity.h"
#include "AK/AK.h"

void Debug::log(std::string const& message, DebugType type)
{
    std::string prefix;

    switch (type)
    {
    case DebugType::Log:
        prefix += "Log: ";
        break;
    case DebugType::Warning:
        prefix += "Warning: ";
        break;
    case DebugType::Error:
        prefix += "Error: ";
        break;
    default:
        std::unreachable();
    }

    debug_messages.emplace_back(type, prefix + message);
}

void Debug::clear()
{
    debug_messages.clear();
}

void Debug::draw_debug_sphere(glm::vec3 const position, float const radius, float const time)
{
    auto const debug_entity = Entity::create("DEBUG_" + AK::generate_guid());
    debug_entity->add_component(DebugDrawing::create(position, radius, time));
}

void Debug::draw_debug_box(glm::vec3 const position, glm::vec3 const euler_angles, glm::vec3 const extents, float const time)
{
    auto const debug_entity = Entity::create("DEBUG_" + AK::generate_guid());
    debug_entity->add_component(DebugDrawing::create(position, euler_angles, extents, time));
}
