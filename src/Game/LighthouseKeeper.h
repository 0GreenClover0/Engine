#pragma once

#include "Component.h"
#include "Input.h"
#include "ParticleSystem.h"

class Port;
class Factory;
class Lighthouse;

enum class WorldPromptType
{
    Port,
    Lighthouse,
    Factory
};

class LighthouseKeeper final : public Component
{
public:
    static std::shared_ptr<LighthouseKeeper> create();

    explicit LighthouseKeeper(AK::Badge<LighthouseKeeper>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;

    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;

    virtual void on_destroyed() override;

    bool is_inside_port() const;
    void set_is_inside_port(bool const value);

    void show_interaction_prompt(glm::vec3 const& position, WorldPromptType type);
    void hide_interaction_prompt(WorldPromptType type);

    glm::vec2 get_speed() const;

    void add_package();
    void remove_package();

    float maximum_speed = 5.0f;
    float acceleration = 0.2f;
    float deceleration = 0.1f;

    NON_SERIALIZED
    float interact_with_factory_distance = 1.0f;

    std::weak_ptr<Lighthouse> lighthouse = {};
    std::weak_ptr<Port> port = {};
    std::weak_ptr<ParticleSystem> keeper_dust = {};
    std::weak_ptr<ParticleSystem> keeper_splash = {};

    std::vector<std::weak_ptr<Entity>> packages = {};

private:
    void handle_input();

    bool m_is_inside_port = false;

    glm::vec2 m_speed = glm::vec2(0.0f, 0.0f);

    float m_package_tilt_x = 0.0f;
    float m_target_package_tilt_x = 0.0f;

    float m_package_tilt_z = 0.0f;
    float m_target_package_tilt_z = 0.0f;

    std::weak_ptr<Entity> m_port_prompt = {};
    std::weak_ptr<Entity> m_lighthouse_prompt = {};
    std::weak_ptr<Entity> m_factory_prompt = {};
};
