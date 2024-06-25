#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"
#include "DialoguePromptController.h"

#include <glm/vec2.hpp>

class GameController final : public Component
{
public:
    static std::shared_ptr<GameController> create();

    explicit GameController(AK::Badge<GameController>);

    static std::shared_ptr<GameController> get_instance();
    u32 get_level_number() const;

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    bool is_moving_to_next_scene() const;
    void move_to_next_scene();

    void reset_level();
    void restart_level();

    std::weak_ptr<Entity> current_scene = {};
    std::weak_ptr<Entity> next_scene = {};
    std::weak_ptr<DialoguePromptController> dialog_manager = {};

private:
    void reset_scene();
    float ease_in_out_cubic(float const x) const;

    void update_scenes_position() const;

    inline static std::shared_ptr<GameController> m_instance;

    bool m_move_to_next_scene = false;
    float m_move_to_next_scene_counter = 0.0f;

    glm::vec2 m_current_position = {};
    glm::vec2 m_next_position = {};
    u32 m_level_number = 0;

    std::vector<std::string> m_levels_order = {"Level_0", "Level_1", "Level_2", "Level_3", "Level_4", "Level_5", "Level_6"};
    std::vector<std::string> m_levels_backup = {};
    std::vector<glm::vec2> m_points_backup = {};
};
