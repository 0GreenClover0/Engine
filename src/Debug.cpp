#include "Debug.h"

#include "AK/AK.h"
#include "DebugDrawing.h"
#include "Entity.h"

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

std::shared_ptr<Entity> Debug::draw_debug_sphere(glm::vec3 const position, float const radius, float const time)
{
    auto debug_entity = Entity::create("DEBUG_" + AK::generate_guid());
    debug_entity->add_component(DebugDrawing::create(position, radius, time));
    debug_entity->is_serialized = false;
    return debug_entity;
}

std::shared_ptr<Entity> Debug::draw_debug_box(glm::vec3 const position, glm::vec3 const euler_angles, glm::vec3 const extents,
                                              float const time)
{
    auto debug_entity = Entity::create("DEBUG_" + AK::generate_guid());
    debug_entity->add_component(DebugDrawing::create(position, euler_angles, extents, time));
    debug_entity->is_serialized = false;
    return debug_entity;
}
