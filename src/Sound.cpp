#include "Sound.h"

#include "Engine.h"
#include "Entity.h"

#if EDITOR
#include <imgui_stdlib.h>
#endif

// TODO: Add an option to load sounds from files at the start of the game

std::shared_ptr<Sound> Sound::create()
{
    auto sound = std::make_shared<Sound>(AK::Badge<Sound> {});

    sound->set_can_tick(true);

    return sound;
}

std::shared_ptr<Sound> Sound::create(std::string const& path)
{
    std::shared_ptr<Sound> sound = std::make_shared<Sound>(AK::Badge<Sound> {});
    ma_sound_init_from_file(&Engine::audio_engine, path.c_str(), 0, nullptr, nullptr, &sound->m_internal_sound);

    sound->set_can_tick(true);

    return sound;
}

std::shared_ptr<Sound> Sound::create(std::string const& path, glm::vec3 const direction, float const rolloff,
                                     ma_attenuation_model const attenuation)
{
    std::shared_ptr<Sound> sound = std::make_shared<Sound>(AK::Badge<Sound> {});
    ma_sound_init_from_file(&Engine::audio_engine, path.c_str(), 0, nullptr, nullptr, &sound->m_internal_sound);

    ma_sound_set_attenuation_model(&sound->m_internal_sound, attenuation);
    ma_sound_set_direction(&sound->m_internal_sound, direction.x, direction.y, direction.z);
    ma_sound_set_rolloff(&sound->m_internal_sound, rolloff);

    sound->set_can_tick(true);
    sound->is_positional = true;

    return sound;
}

std::shared_ptr<Sound> Sound::play_sound(std::string const& path, bool const loop)
{
    auto sound = create(path);
    ma_sound_start(&sound->m_internal_sound);
    ma_sound_set_looping(&sound->m_internal_sound, loop);

    auto const e = Entity::create("TemporarySound");
    e->add_component<Sound>(sound);

    return sound;
}

std::shared_ptr<Sound> Sound::play_sound_at_location(std::string const& path, glm::vec3 const position, glm::vec3 const direction,
                                                     float const rolloff, ma_attenuation_model const attenuation, bool const loop)
{
    auto sound = create(path, direction, rolloff, attenuation);

    ma_sound_set_position(&sound->m_internal_sound, position.x, position.y, position.z);
    ma_sound_start(&sound->m_internal_sound);
    ma_sound_set_looping(&sound->m_internal_sound, loop);

    auto const e = Entity::create("TemporarySound");
    e->transform->set_local_position(position);
    e->add_component<Sound>(sound);

    return sound;
}

void Sound::awake()
{
    if (play_on_awake)
    {
        play();
    }

    if (m_internal_sound.pDataSource != nullptr)
    {
        if (!is_positional)
            ma_sound_set_attenuation_model(&m_internal_sound, ma_attenuation_model_linear);

        ma_sound_set_volume(&m_internal_sound, volume);
    }
}

#if EDITOR
void Sound::draw_editor()
{
    Component::draw_editor();

    if (ImGui::InputText("Sound path", &path))
    {
        reprepare();
    }

    if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f"))
    {
        set_volume(volume);
    }

    ImGui::Checkbox("Play on Awake", &play_on_awake);
}
#endif

void Sound::reprepare()
{
    Component::reprepare();

    if (!is_positional)
    {
        ma_sound_uninit(&m_internal_sound);

        auto const result = ma_sound_init_from_file(&Engine::audio_engine, path.c_str(), 0, nullptr, nullptr, &m_internal_sound);

        if (result != MA_SUCCESS)
        {
            return;
        }

        // NOTE: Setting attenuation model to none makes the ma_sound_set_volume() not work at all, for some reason.
        // ma_sound_set_attenuation_model(&m_internal_sound, ma_attenuation_model_none);

        if (!is_positional)
            ma_sound_set_attenuation_model(&m_internal_sound, ma_attenuation_model_linear);

        ma_sound_set_volume(&m_internal_sound, volume);
    }
}

void Sound::set_position(glm::vec3 const& position)
{
    ma_sound_set_position(&m_internal_sound, position.x, position.y, position.z);
}

void Sound::play()
{
    ma_sound_start(&m_internal_sound);
}

void Sound::stop()
{
    ma_sound_stop(&m_internal_sound);
}

bool Sound::has_finished() const
{
    return m_internal_sound.atEnd;
}

void Sound::stop_with_fade(u64 const milliseconds)
{
    ma_sound_stop_with_fade_in_milliseconds(&m_internal_sound, milliseconds);
}

void Sound::set_volume(float const new_volume)
{
    if (!is_positional)
        ma_sound_set_attenuation_model(&m_internal_sound, ma_attenuation_model_linear);

    ma_sound_set_volume(&m_internal_sound, new_volume);
    volume = new_volume;
}

void Sound::update()
{
    if (is_positional)
    {
        auto const position = entity->transform->get_position();
        ma_sound_set_position(&m_internal_sound, position.x, position.y, position.z);
    }

    // Cleanup if the sound has ended. Note that for looping sounds atEnd is never true.
    if (m_internal_sound.atEnd)
    {
        ma_sound_uninit(&m_internal_sound);

        entity->destroy_immediate();
    }
}
