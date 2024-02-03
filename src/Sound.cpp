#include "Sound.h"

#include "Engine.h"
#include "Entity.h"

// TODO: Add an option to load sounds from files at the start of the game

std::shared_ptr<Sound> Sound::create(std::string const& path)
{
    std::shared_ptr<Sound> sound = std::make_shared<Sound>();
    ma_sound_init_from_file(&Engine::audio_engine, path.c_str(), 0, nullptr, nullptr, &sound->internal_sound);

    ma_sound_set_attenuation_model(&sound->internal_sound, ma_attenuation_model_none);

    sound->set_can_tick(true);

    return sound;
}

std::shared_ptr<Sound> Sound::create(std::string const& path, glm::vec3 const direction, float const rolloff,
                                     ma_attenuation_model const attenuation)
{
    std::shared_ptr<Sound> sound = std::make_shared<Sound>();
    ma_sound_init_from_file(&Engine::audio_engine, path.c_str(), 0, nullptr, nullptr, &sound->internal_sound);

    ma_sound_set_attenuation_model(&sound->internal_sound, attenuation);
    ma_sound_set_direction(&sound->internal_sound, direction.x, direction.y, direction.z);
    ma_sound_set_rolloff(&sound->internal_sound, rolloff);

    sound->set_can_tick(true);
    sound->is_positional = true;

    return sound;
}

std::shared_ptr<Sound> Sound::play_sound(std::string const& path)
{
    auto sound = create(path);
    ma_sound_start(&sound->internal_sound);

    auto const e = Entity::create("TemporarySound");
    e->add_component<Sound>(sound);

    return sound;
}

std::shared_ptr<Sound> Sound::play_sound_at_location(std::string const& path, glm::vec3 const position, glm::vec3 const direction,
                                                     float const rolloff, ma_attenuation_model const attenuation)
{
    auto sound = create(path, direction, rolloff, attenuation);

    ma_sound_set_position(&sound->internal_sound, position.x, position.y, position.z);
    ma_sound_start(&sound->internal_sound);

    auto const e = Entity::create("TemporarySound");
    e->transform->set_local_position(position);
    e->add_component<Sound>(sound);

    return sound;
}

void Sound::play()
{
    ma_sound_start(&internal_sound);
}

void Sound::stop()
{
    ma_sound_stop(&internal_sound);
}

void Sound::stop_with_fade(uint64_t const milliseconds)
{
    ma_sound_stop_with_fade_in_milliseconds(&internal_sound, milliseconds);
}

void Sound::update()
{
    if (is_positional)
    {
        auto const position = entity->transform->get_position();
        ma_sound_set_position(&internal_sound, position.x, position.y, position.z);
    }

    // Cleanup if the sound has ended. Note that for looping sounds atEnd is never true.
    if (internal_sound.atEnd)
    {
        ma_sound_uninit(&internal_sound);
        set_can_tick(false);

        // TODO: Destroy the entity
    }
}
