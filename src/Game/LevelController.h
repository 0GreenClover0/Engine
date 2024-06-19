#pragma once

#include "Component.h"
#include "Curve.h"
#include "Engine.h"
#include "Factory.h"
#include "Lighthouse.h"
#include "Player.h"
#include "Port.h"

class ScreenText;
class Factory;
class Lighthouse;

enum class TutorialProgressAction
{
    PackageCollected,
    ShipEnteredPort,
    ShipEnteredControl,
    ShipDestroyed,
    KeeperLeavedLighthouse,
    KeeperEnteredLighthouse,
    KeeperEnteredPort,
    LighthouseEnabled,
    GeneratorFueled,
    WorkshopUpgraded,
};

class LevelController final : public Component
{
public:
    static std::shared_ptr<LevelController> create();

    static std::shared_ptr<LevelController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    void on_lighthouse_upgraded() const;

    void set_exiting_lighthouse(bool value);
    bool get_exiting_lighthouse();

    void check_tutorial_progress(TutorialProgressAction action);

    void progress_tutorial(i32 step = 1);

    NON_SERIALIZED
    bool is_started = false;

    float map_time = 180.0f;
    u32 map_food = 20;
    i32 maximum_lighthouse_level = 10;
    NON_SERIALIZED
    float time = 0.0f;

    std::vector<std::weak_ptr<Factory>> factories = {};
    std::weak_ptr<Port> port = {};
    std::weak_ptr<Lighthouse> lighthouse = {};

    float playfield_width = 6.5f;
    float playfield_additional_width = 2.0f;
    float playfield_height = 4.8f;
    float playfield_y_shift = -2.1f;

    std::weak_ptr<Curve> ships_limit_curve = {};
    u32 ships_limit = 0;

    std::weak_ptr<Curve> ships_speed_curve = {};
    float ships_speed = 0.0f;

    std::weak_ptr<Curve> ships_range_curve = {};
    std::weak_ptr<Curve> ships_turn_curve = {};
    std::weak_ptr<Curve> ships_additional_speed_curve = {};
    std::weak_ptr<Curve> pirates_in_control_curve = {};

    bool is_tutorial = false;
    u32 starting_packages = 2;
    NON_SERIALIZED
    u32 tutorial_progress = 0;
    u32 tutorial_level = 1;

private:
    inline static std::shared_ptr<LevelController> m_instance;

    std::weak_ptr<Player> player_ref = {};
    std::weak_ptr<ScreenText> clock_text_ref = {};
    std::weak_ptr<ScreenText> m_text = {};

    bool m_is_exiting_lighthouse_enabled = true;
};
