#include "SoundListener.h"

#include <iostream>

#include "Engine.h"
#include "Entity.h"

std::shared_ptr<SoundListener> SoundListener::create()
{
    auto sound_listener = std::make_shared<SoundListener>(AK::Badge<SoundListener> {});

    if (instance != nullptr)
    {
        std::cout << "Error: SoundListener already exists in the scene.";
    }
    else
    {
        instance = sound_listener;
        sound_listener->set_can_tick(true);
    }

    return sound_listener;
}

void SoundListener::update()
{
    glm::vec3 const position = entity->transform->get_position();
    glm::vec3 const forward = entity->transform->get_forward();

    ma_engine_listener_set_position(&Engine::audio_engine, 0, position.x, position.y, position.z);
    ma_engine_listener_set_direction(&Engine::audio_engine, 0, forward.x, forward.y, forward.z);
}
