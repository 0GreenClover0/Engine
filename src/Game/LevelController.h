#pragma once

#include "Component.h"
#include "Engine.h"

class ScreenText;
class Factory;

class LevelController final : public Component
{
public:
    static std::shared_ptr<LevelController> create();

    static std::shared_ptr<LevelController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    void on_lighthouse_upgraded();

    float map_time = 0.0f;
    NON_SERIALIZED
    float time = 0.0f;

    i32 ships_limit = 3;

    std::vector<std::weak_ptr<Factory>> factories = {};

    float const playfield_width = 5.4f;
    float const playfield_additional_width = 1.4f;
    float const playfield_height = 3.6f;

    //TODO KRZYWE

private:
    inline static std::shared_ptr<LevelController> m_instance;

    std::weak_ptr<ScreenText> m_text = {};
};
