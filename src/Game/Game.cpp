#include "Game.h"

#include <glm/gtc/random.hpp>

#include "Camera.h"
#include "Collider2D.h"
#include "CommonEntities.h"
#include "Cube.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "Sprite.h"
#include "ExampleDynamicText.h"
#include "Grass.h"
#include "ExampleUIBar.h"
#include "MeshFactory.h"
#include "Model.h"
#include "PointLight.h"
#include "ResourceManager.h"
#include "ScreenText.h"
#include "ShaderFactory.h"
#include "Sound.h"
#include "SoundListener.h"
#include "SpotLight.h"
#include "Terrain.h"
#include "Player/PlayerInput.h"
#include "LighthouseKeeper.h"
#include "LighthouseLight.h"
#include "Sphere.h"
#include "Ship.h"
#include "GameController.h"

Game::Game(std::shared_ptr<Window> const& window) : window(window)
{
}

void Game::initialize()
{
    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const plain_shader = ResourceManager::get_instance().load_shader("./res/shaders/simple.hlsl", "./res/shaders/simple.hlsl");
    auto const light_source_shader = ResourceManager::get_instance().load_shader("./res/shaders/light_source.hlsl", "./res/shaders/light_source.hlsl");

    auto const standard_material = Material::create(standard_shader);
    auto const plain_material = Material::create(plain_shader);
    auto const light_source_material = Material::create(light_source_shader);

    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, 1);

    m_camera = Entity::create("Camera");
    m_camera->transform->set_local_position(glm::vec3(0.0f, 11.3f, 11.3f));
    m_camera->transform->set_euler_angles(glm::vec3(-45.0f, 0.0f, 0.0f));
    m_camera->add_component<SoundListener>(SoundListener::create());

    m_camera_comp = m_camera->add_component(Camera::create());
    m_camera_comp->set_can_tick(true);
    m_camera_comp->set_fov(glm::radians(17.51f));
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
    directional_light->transform->set_local_position({ 0.0f, 14.0f, 0.0f });
    directional_light->transform->set_euler_angles({ -90.0f, 0.0f, 0.0f });

    auto const directional_light_component = directional_light->add_component<DirectionalLight>(DirectionalLight::create());

    directional_light_component->ambient = glm::vec3(1.0f);
    directional_light_component->diffuse = glm::vec3(1.0f);
    directional_light_component->specular = glm::vec3(1.0f);

    auto const scene = Entity::create("scene");
    scene->add_component(Model::create("./res/models/scene/scene.gltf", standard_material));

    auto const water = Entity::create("water");
    water->add_component(Model::create("./res/models/water/water.gltf", standard_material));

    auto const keeper = Entity::create("keeper");
    keeper->add_component(Model::create("./res/models/keeper/keeper.gltf", standard_material));
    keeper->add_component(LighthouseKeeper::create());
    keeper->transform->set_local_position({ 0.0f, 0.0f, 3.0f });

    auto const light = Entity::create("light");
    light->add_component(Sphere::create(0.25f, 12, 12, "./res/textures/stone.jpg", standard_material));
    light->add_component(LighthouseLight::create());;

    auto const ship = Entity::create("ship");
    ship->add_component(Model::create("./res/models/ship/ship.gltf", standard_material));
    ship->transform->set_local_position({ -5.5f, 0.0f, 0.0f });
    ship->add_component(Ship::create(light->get_component<LighthouseLight>()));;

    auto const game_controller = Entity::create("game_controller");
    game_controller->add_component(GameController::create());
}
