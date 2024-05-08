#pragma once

#include "Component.h"
#include "Engine.h"
#include "Curve.h"

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

    float map_time = 180.0f;
    NON_SERIALIZED
    float time = 0.0f;

    std::vector<std::weak_ptr<Factory>> factories = {};

    float const playfield_width = 5.4f;
    float const playfield_additional_width = 1.4f;
    float const playfield_height = 3.6f;

    std::weak_ptr<Curve> ships_limit_curve = {};
    u32 ships_limit = 0;

private:
    inline static std::shared_ptr<LevelController> m_instance;

    std::weak_ptr<ScreenText> m_text = {};
};
