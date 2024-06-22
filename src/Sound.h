#pragma once

#include <glm/vec3.hpp>
#include <miniaudio.h>

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

class Sound final : public Component
{
public:
    static std::shared_ptr<Sound> create();
    static std::shared_ptr<Sound> create(std::string const& path);
    static std::shared_ptr<Sound> create(std::string const& path, glm::vec3 const direction, float const rolloff = 0.5f,
                                         ma_attenuation_model const attenuation = ma_attenuation_model_inverse);
    static std::shared_ptr<Sound> play_sound(std::string const& path);
    static std::shared_ptr<Sound> play_sound_at_location(std::string const& path, glm::vec3 const position, glm::vec3 direction,
                                                         float rolloff = 0.5f,
                                                         ma_attenuation_model attenuation = ma_attenuation_model_inverse);

    explicit Sound(AK::Badge<Sound>)
    {
    }

    virtual void awake() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void reprepare() override;

    void play();
    void stop();
    bool has_finished() const;
    void stop_with_fade(u64 const milliseconds);
    void set_volume(float const new_volume);
    virtual void update() override;

    std::string path = {};

    float volume = 10.0f;
    bool play_on_awake = false;
    bool is_positional = false;

private:
    ma_sound m_internal_sound = {};
};
