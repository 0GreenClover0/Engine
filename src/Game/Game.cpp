#include "Game.h"

#include <glm/gtc/random.hpp>

#include "Camera.h"
#include "CommonEntities.h"
#include "Cube.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "Sprite.h"
#include "MeshFactory.h"
#include "Model.h"
#include "PointLight.h"
#include "ShaderFactory.h"
#include "Sound.h"
#include "SoundListener.h"
#include "SpotLight.h"
#include "Terrain.h"
#include "Player/PlayerInput.h"

Game::Game(std::shared_ptr<Window> const& window) : window(window)
{
}

void Game::initialize()
{
    auto standard_shader = ShaderFactory::create("./res/shaders/simple.hlsl", "./res/shaders/simple.hlsl");
    auto ui_shader = ShaderFactory::create("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");

    auto const standard_material = Material::create(standard_shader);
    auto const ui_material = Material::create(ui_shader);

    m_camera = Entity::create("Camera");
    m_camera->transform->set_local_position(glm::vec3(0.0f, 0.0f, 0.0f));

    m_camera->add_component<SoundListener>(SoundListener::create());

    m_camera_comp = m_camera->add_component(Camera::create());
    m_camera_comp->set_can_tick(true);
    m_camera_comp->set_fov(glm::radians(60.0f));

    auto const player = Entity::create("Player");
    auto const player_input = player->add_component<PlayerInput>();
    player_input->set_can_tick(true);
    player_input->camera_entity = m_camera;
    player_input->player = player;
    player_input->window = window;

    auto const camera_parent = Entity::create("CameraParent");
    camera_parent->transform->set_parent(player->transform);
    m_camera->transform->set_parent(camera_parent->transform);
    player_input->camera_parent = camera_parent;

    auto const model = Entity::create("testmodel");
    model->add_component(Model::create("./res/models/pyramid3/scene.gltf", standard_material));
    model->transform->set_local_position(glm::vec3(0.0f, 0.0f, -5.0f));
    
    auto const sprite = Entity::create("testSprite");

    sprite->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/grass.png"));
    sprite->transform->set_local_scale({ 0.5f, 0.5f, 1.0f });
}
