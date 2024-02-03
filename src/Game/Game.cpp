#include "Game.h"

#include <glm/gtc/random.hpp>

#include "Camera.h"
#include "CommonEntities.h"
#include "Cube.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "Grass.h"
#include "Model.h"
#include "PointLight.h"
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
    auto instanced_shader = Shader::create("./res/shaders/standard_instanced.vert", "./res/shaders/standard.frag");
    auto standard_shader = Shader::create("./res/shaders/standard.vert", "./res/shaders/standard.frag");
    auto reflective_shader = Shader::create("./res/shaders/standard.vert", "./res/shaders/reflective.frag");
    auto refractive_shader = Shader::create("./res/shaders/standard.vert", "./res/shaders/refractive.frag");
    auto const standard_material = Material::create(standard_shader);

    camera = Entity::create("Camera");
    camera->transform->set_local_position(glm::vec3(0.0f, 0.0f, 10.0f));

    camera->add_component<SoundListener>(SoundListener::create());

    camera_comp = camera->add_component(Camera::create());
    camera_comp->set_can_tick(true);
    camera_comp->set_fov(glm::radians(60.0f));

    auto const player = Entity::create("Player");
    auto const player_input = player->add_component<PlayerInput>();
    player_input->set_can_tick(true);
    player_input->camera_entity = camera;
    player_input->player = player;
    player_input->window = window;

    auto const player_model = Entity::create("PlayerModel");
    player_model->transform->set_parent(player->transform);
    player_model->transform->set_euler_angles(glm::vec3(0.0f, 180.0f, 0.0f));
    player_model->transform->set_local_scale(glm::vec3(0.01f, 0.01f, 0.01f));

    auto const camera_parent = Entity::create("CameraParent");
    camera_parent->transform->set_parent(player->transform);
    camera->transform->set_parent(camera_parent->transform);
    player_input->camera_parent = camera_parent;

    auto terminator_material = Material::create(standard_shader);
    auto const player_body = Entity::create("Body");
    player_body->transform->set_parent(player_model->transform);
    player_body->add_component<Model>(Model::create("./res/models/terminator2/Terminator2.obj", terminator_material));

    auto const reflective_material = Material::create(reflective_shader);
    reflective_material->needs_skybox = true;
    auto const left_hand = Entity::create("LeftHand");
    left_hand->transform->set_parent(player_model->transform);
    left_hand->add_component<Model>(Model::create("./res/models/terminator2/LeftHand.obj", reflective_material));

    auto const refractive_material = Material::create(refractive_shader);
    refractive_material->needs_skybox = true;
    auto const right_hand = Entity::create("RightHand");
    right_hand->transform->set_parent(player_model->transform);
    right_hand->add_component<Model>(Model::create("./res/models/terminator2/RightHand.obj", refractive_material));

    auto const head = Entity::create("Head");
    head->transform->set_parent(player_model->transform);
    head->transform->set_local_position(glm::vec3(0.0f, 185.0f, 0.0f));
    head->add_component<Model>(Model::create("./res/models/terminator2/Head.obj", terminator_material));
    player_input->player_head = head;
    player_input->player_model = player_model;

    auto const reflective_cube = Entity::create("Reflective");
    reflective_cube->add_component<Cube>(Cube::create(reflective_material));
    reflective_cube->transform->set_local_position(glm::vec3(-5.0f, 1.0f, 0.0f));
    reflective_cube->transform->set_local_scale(glm::vec3(2.0f, 2.0f, 2.0f));

    auto const refractive_cube = Entity::create("Refractive");
    refractive_cube->add_component<Cube>(Cube::create(refractive_material));
    refractive_cube->transform->set_local_position(glm::vec3(5.0f, 1.0f, 0.0f));
    refractive_cube->transform->set_local_scale(glm::vec3(2.0f, 2.0f, 2.0f));

    auto const grass_material = Material::create(instanced_shader, 101, true, true);

    auto const grass_parent = Entity::create("Grass");
    for (uint32_t i = 0; i < 10000; ++i)
    {
        auto const grass = Entity::create("Grass");
        grass->transform->set_parent(grass_parent->transform);
        grass->add_component<Grass>(Grass::create(grass_material, 1, "./res/textures/grass.png"));
        grass->transform->set_local_position(glm::vec3(glm::linearRand(-50.0f, 50.0f), 0.0f, glm::linearRand(-50.0f, 50.0f)));
    }

    auto const root = Entity::create("Root");

    auto const point_light = CommonEntities::create_point_light(glm::vec3(1.0f, 1.0f, 0.0f), root->transform);
    auto const point_light_comp = point_light->get_component<PointLight>();
    auto const point_light_material = point_light->get_component<Cube>()->material;

    auto const directional_light = CommonEntities::create_directional_light(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-65.0f, -1.0f, 0.1f), root->transform);
    directional_light->transform->set_local_position(glm::vec3(0.0f, 5.0f, 0.0f));
    auto const directional_light_comp = directional_light->get_component<DirectionalLight>();
    auto const directional_light_material = directional_light->get_component<Cube>()->material;

    auto const directional_arrow = Entity::create("DirectionalArrow");
    directional_arrow->transform->set_parent(directional_light->transform);
    directional_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    directional_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", directional_light_material));

    auto const spot_light = CommonEntities::create_spot_light(glm::vec3(0.0f, 0.2f, 1.0f), root->transform);
    auto const spot_light_comp = spot_light->get_component<SpotLight>();
    auto const spot_light_material = spot_light->get_component<Cube>()->material;
    spot_light->transform->set_local_position(glm::vec3(12.0f, 0.0f, 0.0f));
    spot_light->transform->set_euler_angles(glm::vec3(-2.0f, 90.0f, -2.0f));

    auto const spot_arrow = Entity::create("SpotArrow");
    spot_arrow->transform->set_parent(spot_light->transform);
    spot_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    spot_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", spot_light_material));

    auto const spot_light2 = CommonEntities::create_spot_light(glm::vec3(1.0f, 0.0f, 0.0f), root->transform);
    auto const spot_light2_comp = spot_light2->get_component<SpotLight>();
    auto const spot_light2_material = spot_light2->get_component<Cube>()->material;
    spot_light2->transform->set_local_position(glm::vec3(-13.0f, 0.0f, 0.0f));
    spot_light2->transform->set_euler_angles(glm::vec3(2.0f, 0.0f, -2.0f));

    auto const spot2_arrow = Entity::create("SpotArrow");
    spot2_arrow->transform->set_parent(spot_light2->transform);
    spot2_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    spot2_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", spot_light2_material));

    auto const cube_material = Material::create(instanced_shader, 0, true);
    auto const roof_material = Material::create(instanced_shader, 0, true);
    auto const floor_material = Material::create(standard_shader);

    auto terrain_shader = Shader::create("./res/shaders/terrain.vert", "./res/shaders/terrain.tcs", "./res/shaders/terrain.tes", "./res/shaders/terrain.frag");
    auto const terrain_material = Material::create(terrain_shader);
    terrain_material->needs_view_model = true;

    auto const terrain = Entity::create("Terrain");
    terrain->transform->set_local_position(glm::vec3(0.0f, 11.0f, 0.0f));
    terrain->add_component<Terrain>(Terrain::create(terrain_material, true, "./res/textures/map/italy_height.png"));

    //auto const floor = Entity::create("Floor");
    //floor->transform->set_parent(root->transform);
    //floor->add_component<Cube>(Cube::create("./res/textures/stone.jpg", floor_material, true));
    //floor->transform->set_local_scale(glm::vec3(1000.0f, 0.1f, 1000.0f));
    //floor->transform->set_local_position(glm::vec3(0.0f, -1.0f, 0.0f));

    float house_x = 0.0f;
    float house_z = 0.0f;
    for (int32_t i = 0; i < 40000; ++i)
    {
        auto const house = CommonEntities::create_cube("House" + std::to_string(i), "./res/textures/container.png", "./res/textures/container_specular.png", cube_material);
        house->transform->set_parent(root->transform);
        house->transform->set_local_position(glm::vec3(house_x, 0.0f, house_z));
        house->transform->set_local_scale(glm::vec3(2.0f, 2.0f, 2.0f));
        house_x += 10.0f;

        auto const roof = Entity::create("Roof" + std::to_string(i));
        roof->add_component<Model>(Model::create("./res/models/pyramid_custom/pyramid.gltf", roof_material));
        roof->transform->set_parent(house->transform);
        roof->transform->set_local_position(glm::vec3(0.0f, 0.6f, 0.0f));
        roof->transform->set_local_scale(glm::vec3(0.4f, 0.4f, 0.4f));

        if (house_x >= 300.0f)
        {
            house_x = -300.0f;
            house_z += 10.0f;
        }
    }

    std::vector<std::string> const skybox_texture_paths =
    {
        "./res/textures/skybox/interstellar/interstellar_ft.tga",
        "./res/textures/skybox/interstellar/interstellar_bk.tga",
        "./res/textures/skybox/interstellar/interstellar_up.tga",
        "./res/textures/skybox/interstellar/interstellar_dn.tga",
        "./res/textures/skybox/interstellar/interstellar_rt.tga",
        "./res/textures/skybox/interstellar/interstellar_lf.tga",
    };
    auto const skybox = CommonEntities::create_skybox(skybox_texture_paths, root->transform);

    //auto const material_backpack = Material::create(instanced_shader, 0, true);
    //auto const back = Entity::create("backpck");
    //back->transform->set_local_position(glm::vec3(0.0f, 0.0f, -3.0f));
    //back->add_component<Model>(Model::create("./res/models/backpack/backpack.obj", material_backpack));
}
