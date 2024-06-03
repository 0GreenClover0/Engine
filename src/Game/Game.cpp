#include "Game.h"

#include <glm/gtc/random.hpp>

#include "Camera.h"
#include "Collider2D.h"
#include "CommonEntities.h"
#include "Cube.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "ExampleDynamicText.h"
#include "ExampleUIBar.h"
#include "Factory.h"
#include "Grass.h"
#include "LevelController.h"
#include "Lighthouse.h"
#include "LighthouseKeeper.h"
#include "LighthouseLight.h"
#include "MeshFactory.h"
#include "Model.h"
#include "Player.h"
#include "Player/PlayerInput.h"
#include "PointLight.h"
#include "Port.h"
#include "ResourceManager.h"
#include "ScreenText.h"
#include "ShaderFactory.h"
#include "Ship.h"
#include "ShipSpawner.h"
#include "Skybox.h"
#include "SkyboxFactory.h"
#include "Sound.h"
#include "SoundListener.h"
#include "Sphere.h"
#include "SpotLight.h"
#include "Sprite.h"
#include "Terrain.h"
#include "Water.h"

Game::Game(std::shared_ptr<Window> const& window) : window(window)
{
}

void Game::initialize()
{
    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const plain_shader = ResourceManager::get_instance().load_shader("./res/shaders/simple.hlsl", "./res/shaders/simple.hlsl");
    auto const light_source_shader =
        ResourceManager::get_instance().load_shader("./res/shaders/light_source.hlsl", "./res/shaders/light_source.hlsl");
    auto const halo_shader = ResourceManager::get_instance().load_shader("./res/shaders/halo.hlsl", "./res/shaders/halo.hlsl");

    auto const standard_material = Material::create(standard_shader);
    auto const plain_material = Material::create(plain_shader);
    auto const light_source_material = Material::create(light_source_shader);
    auto const halo_material = Material::create(halo_shader);

    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, 1);
    ui_material->casts_shadows = false;

    m_camera = Entity::create("Camera");
    m_camera->transform->set_local_position(glm::vec3(0.0f, 17.0f, 13.0f));
    m_camera->transform->set_euler_angles(glm::vec3(-50.0f, 0.0f, 0.0f));
    m_camera->add_component<SoundListener>(SoundListener::create());

    m_camera_comp = m_camera->add_component(Camera::create());
    m_camera_comp->set_can_tick(true);
    m_camera_comp->set_fov(glm::radians(20.0f));
    m_camera_comp->update();

    auto const player = Entity::create("Player");
    auto const player_input = player->add_component<PlayerInput>(PlayerInput::create());
    player_input->set_can_tick(true);
    player_input->camera_entity = m_camera;
    player_input->player = player;
    player_input->window = window;

    auto const camera_parent = Entity::create("CameraParent");
    camera_parent->transform->set_parent(player->transform);
    m_camera->transform->set_parent(camera_parent->transform);
    player_input->camera_parent = camera_parent;

    auto const directional_light = Entity::create("Directional light");
    directional_light->add_component(Sphere::create(0.1f, 10, 10, "./res/textures/container.png", light_source_material));
    directional_light->transform->set_local_position({0.0f, 14.0f, 0.0f});
    directional_light->transform->set_euler_angles({-90.0f, 0.0f, 0.0f});

    auto const directional_light_component = directional_light->add_component<DirectionalLight>(DirectionalLight::create());

    directional_light_component->ambient = glm::vec3(1.0f);
    directional_light_component->diffuse = glm::vec3(1.0f);
    directional_light_component->specular = glm::vec3(1.0f);

    auto const scene = Entity::create("Scene");
    scene->add_component(Model::create("./res/models/scene/scene.gltf", standard_material));

    auto const port = Entity::create("Port");
    auto const port_comp = port->add_component<Port>(Port::create());
    port->transform->set_local_position({-0.113047f, -0.25f, 2.162429f});
    port->transform->set_local_scale({3.5f, 0.323763f, 1.0f});
    port->add_component(Cube::create("./res/textures/skybox/interstellar/interstellar_bk.tga", standard_material));
    auto const collider = port->add_component<Collider2D>(
        Collider2D::create({port->transform->get_local_scale().x / 2.0f, port->transform->get_local_scale().z / 2.0f}, true));
    collider->set_is_trigger(true);

    auto const generator = Entity::create("Generator");
    auto const generator_comp = generator->add_component<Factory>(Factory::create());
    generator_comp->set_type(FactoryType::Generator);
    generator->transform->set_local_position({3.0f, 1.0f, 3.0f});

    auto const workshop = Entity::create("Workshop");
    auto const workshop_comp = workshop->add_component<Factory>(Factory::create());
    workshop_comp->set_type(FactoryType::Workshop);
    workshop->transform->set_local_position({-3.0f, 0.0f, 3.0f});

#pragma region Lighthouse
    auto const lighthouse = Entity::create("Lighthouse");
    lighthouse->add_component<Model>(Model::create("./res/models/lighthouseNew/lighthouse.gltf", standard_material));
    lighthouse->transform->set_local_position(glm::vec3(2.294563f, 0.223798f, 2.378702f));
    auto const lighthouse_comp = lighthouse->add_component<Lighthouse>(Lighthouse::create());

    auto const lighthouse_light = Entity::create("Lighthouse Light");
    auto const lighthouse_light_comp = lighthouse_light->add_component(LighthouseLight::create());

    auto const spotlight = Entity::create("Lighthouse Spotlight");
    auto const spotlight_comp = spotlight->add_component<SpotLight>(SpotLight::create());

    spotlight->transform->set_parent(lighthouse->transform);
    spotlight->transform->set_local_position(glm::vec3(0.0f, 0.876f, 0.0f));

    lighthouse_light_comp->set_spot_light(spotlight_comp);

    lighthouse_comp->light = lighthouse_light_comp;
    lighthouse_comp->port = port_comp;

    auto const spawn = Entity::create("Spawn");
    spawn->transform->set_parent(lighthouse->transform);
    spawn->transform->set_local_position(glm::vec3(0.0f, 0.0f, 0.4f));
    lighthouse_comp->spawn_position = spawn;

#pragma endregion

#pragma region Controller
    auto const controller = Entity::create("Controller");
    auto const controller_comp = controller->add_component(LevelController::create());
    auto const ship_spawner_comp = controller->add_component(ShipSpawner::create(lighthouse_light->get_component<LighthouseLight>()));
    controller->add_component(Player::create());

    controller_comp->ships_limit_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->ships_limit_curve.lock()->custom_name = "Ships limit";
    controller_comp->ships_limit_curve.lock()->add_points({{0.0f, 2.0f}, {0.1f, 2.0f}, {0.6f, 6.0f}, {1.0f, 6.0f}});

    controller_comp->ships_speed_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->ships_speed_curve.lock()->custom_name = "Ships speed";
    controller_comp->ships_speed_curve.lock()->add_points({{0.0f, 0.21f}, {0.756f, 0.26f}, {1.0f, 0.26f}});

    controller_comp->ships_range_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->ships_range_curve.lock()->custom_name = "Ships range";
    controller_comp->ships_range_curve.lock()->add_points({{0.0f, 0.4f}, {0.5f, 0.6f}, {1.0f, 1.5f}});

    controller_comp->ships_turn_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->ships_turn_curve.lock()->custom_name = "Ships turn";
    controller_comp->ships_turn_curve.lock()->add_points({{0.0f, 15.0f}, {1.0f, 60.0f}});

    controller_comp->ships_additional_speed_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->ships_additional_speed_curve.lock()->custom_name = "Ships additional speed";
    controller_comp->ships_additional_speed_curve.lock()->add_points({{0.0f, 0.001f}, {0.2f, 0.0025f}, {1.0f, 0.01f}});

    controller_comp->pirates_in_control_curve = controller->add_component<Curve>(Curve::create());
    controller_comp->pirates_in_control_curve.lock()->custom_name = "Pirates in control";
    controller_comp->pirates_in_control_curve.lock()->add_points({{0.0f, 0.16f}, {1.0f, 1.66f}});

    controller_comp->factories.emplace_back(generator_comp);
    controller_comp->factories.emplace_back(workshop_comp);

    auto const path1 = controller->add_component<Path>(Path::create());
    path1->add_points({{-5.0f, -2.5f}, {-5.0f, 1.7f}});

    auto const path2 = controller->add_component<Path>(Path::create());
    path2->add_points({{5.0f, -2.5f}, {5.0f, 1.7f}});

#pragma endregion

    auto const water = Entity::create("Water");
    auto const water_comp = water->add_component(Water::create());
    water_comp->add_wave();
    water_comp->add_wave();
    water_comp->waves[0].direction = glm::vec2(1.0f, 0.5f);
    water_comp->waves[0].speed = 116.0f;
    water_comp->waves[0].steepness = 0.7f;
    water_comp->waves[0].wave_length = 115.0f;
    water_comp->waves[0].amplitude = 0.04f;

    water_comp->waves[1].direction = glm::vec2(-0.6f, 0.1f);
    water_comp->waves[1].speed = 100.0f;
    water_comp->waves[1].steepness = 0.9f;
    water_comp->waves[1].wave_length = 500.0f;
    water_comp->waves[1].amplitude = 0.035f;

    water->transform->set_local_scale(glm::vec3(0.27f, 1.0f, 0.172f));

    auto const skybox = Entity::create("Skybox");
    skybox->add_component(SkyboxFactory::create());

    auto const sea_floor = Entity::create("Sea Floor");
    sea_floor->transform->set_local_position({0.0f, -0.3f, 0.0f});
    sea_floor->add_component(Model::create("./res/models/water/water.gltf", standard_material));
}
