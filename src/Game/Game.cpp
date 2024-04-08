#include "Game.h"

#include <glm/gtc/random.hpp>

#include "Camera.h"
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
#include "ScreenText.h"
#include "ShaderFactory.h"
#include "Sound.h"
#include "SoundListener.h"
#include "SpotLight.h"
#include "Terrain.h"
#include "Player/PlayerInput.h"
#include <Sphere.h>

Game::Game(std::shared_ptr<Window> const& window) : window(window)
{
}

void Game::initialize()
{
    auto const standard_shader = ShaderFactory::create("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const plain_shader = ShaderFactory::create("./res/shaders/simple.hlsl", "./res/shaders/simple.hlsl");
    auto const light_source_shader = ShaderFactory::create("./res/shaders/light_source.hlsl", "./res/shaders/light_source.hlsl");

    auto const standard_material = Material::create(standard_shader);
    auto const plain_material = Material::create(plain_shader);
    auto const light_source_material = Material::create(light_source_shader);

    auto const ui_shader = ShaderFactory::create("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, 1);

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

    auto const model = Entity::create("testmodel1");
    model->add_component(Model::create("./res/models/pyramid3/scene.gltf", standard_material));
    model->transform->set_local_position(glm::vec3(0.0f, 0.0f, -5.0f));
    
    auto const text = Entity::create("text");
    std::shared_ptr<ExampleDynamicText> const edt = text->add_component<ExampleDynamicText>();

    auto const bar = Entity::create("bar");
    std::shared_ptr<ExampleUIBar> const eub = bar->add_component<ExampleUIBar>();

    auto const transparent = Entity::create("transparent_grass");
    std::shared_ptr<Sprite> const grass = transparent->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/grass.png"));
    transparent->transform->set_local_scale(glm::vec3(0.2f, 0.2f, 0.2f));

    auto const model2 = Entity::create("testmodel2");
    model2->add_component(Model::create("./res/models/pyramid3/scene.gltf", standard_material));
    model2->transform->set_parent(model->transform);


    auto const point_light = Entity::create("Point light");
    point_light->add_component(std::make_shared<Sphere>(0.1f, 10, 10, "./res/textures/container.png", light_source_material));
    point_light->transform->set_local_position(glm::vec3(2.0f, 2.0f, 2.0f));

    auto const point_light_component = point_light->add_component<PointLight>(PointLight::create());
    point_light_component->ambient = glm::vec3(0.05f);
    point_light_component->diffuse = glm::vec3(0.2f);
    point_light_component->specular = glm::vec3(0.2f);
    point_light_component->constant = 1.0f;
    point_light_component->linear = 0.09f;
    point_light_component->quadratic = 0.032f;


    auto const spot_light = Entity::create("Spot light");
    spot_light->add_component(std::make_shared<Sphere>(0.1f, 10, 10, "./res/textures/container.png", light_source_material));
    spot_light->transform->set_local_position(glm::vec3(0.0f, 0.0f, -10.0f));
    spot_light->transform->set_euler_angles(glm::vec3(180.0f, 0.0f, 0.0f));

    auto const spot_light_component = spot_light->add_component<SpotLight>(SpotLight::create());
    spot_light_component->ambient = glm::vec3(0.0f);
    spot_light_component->diffuse = glm::vec3(1.0f);
    spot_light_component->specular = glm::vec3(1.0f);
    spot_light_component->constant = 1.0f;
    spot_light_component->linear = 0.09f;
    spot_light_component->quadratic = 0.032f;
    spot_light_component->cut_off = glm::cos(glm::radians(12.5f));
    spot_light_component->outer_cut_off = glm::cos(glm::radians(15.0f));


    auto const directional_light = Entity::create("Directional light");
    directional_light->add_component(std::make_shared<Sphere>(0.1f, 10, 10, "./res/textures/container.png", light_source_material));
    directional_light->transform->set_local_position({ 0.0f, 0.0f, 1.0f });

    auto const directional_light_component = directional_light->add_component<DirectionalLight>(DirectionalLight::create());
    directional_light_component->ambient = glm::vec3(0.05f);
    directional_light_component->diffuse = glm::vec3(0.4f);
    directional_light_component->specular = glm::vec3(0.5f);
}
