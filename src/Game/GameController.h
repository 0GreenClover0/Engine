#pragma once

#include "Component.h"

class GameController final : public Component
{
public:
    static std::shared_ptr<GameController> create();

    explicit GameController(AK::Badge<GameController>);

    static std::shared_ptr<GameController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    std::weak_ptr<Entity> current_scene = {};
    std::weak_ptr<Entity> next_scene = {};

private:
    float ease_in_out_cubic(float const x) const;

    inline static std::shared_ptr<GameController> m_instance;

    bool m_move_to_next_scene = false;
    float m_move_to_next_scene_counter = 0.0f;
};
