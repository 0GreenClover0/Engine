#include "CommonEntities.h"

#include "Cube.h"
#include "DirectionalLight.h"
#include "Globals.h"
#include "Material.h"
#include "PointLight.h"
#include "ShaderFactory.h"
#include "Skybox.h"
#include "SkyboxFactory.h"
#include "SpotLight.h"

namespace CommonEntities
{

std::shared_ptr<Entity> create_skybox(std::vector<std::string> const& skybox_texture_paths, std::shared_ptr<Transform> const& parent)
{
    auto skybox = Entity::create("Skybox");
    auto const skybox_shader = ShaderFactory::create("./res/shaders/skybox.vert", "./res/shaders/skybox.frag");
    auto const skybox_material = Material::create(skybox_shader, SKYBOX_RENDER_ORDER);

    auto const skybox_comp = skybox->add_component<Skybox>(SkyboxFactory::create(skybox_material, skybox_texture_paths));
    Skybox::set_instance(skybox_comp);
    skybox->transform->set_parent(parent);

    return skybox;
}

std::shared_ptr<Entity> create_directional_light(glm::vec3 const& diffuse, glm::vec3 const& angles, std::shared_ptr<Transform> const& parent)
{
    auto light_shader = ShaderFactory::create("./res/shaders/light.vert", "./res/shaders/light.frag");
    auto const light_material = Material::create(light_shader);

    auto directional_light = Entity::create("DirectionalLight");
    auto const directional_light_comp = directional_light->add_component<DirectionalLight>(DirectionalLight::create());
    directional_light_comp->diffuse = diffuse;
    light_material->color = glm::vec4(diffuse.x, diffuse.y, diffuse.z, 1.0f);
    directional_light->add_component<Cube>(Cube::create(light_material));
    directional_light->transform->set_euler_angles(angles);
    directional_light->transform->set_parent(parent);

    return directional_light;
}

std::shared_ptr<Entity> create_point_light(glm::vec3 const& diffuse, std::shared_ptr<Transform> const& parent)
{
    auto light_shader = ShaderFactory::create("./res/shaders/light.vert", "./res/shaders/light.frag");
    auto const light_material = Material::create(light_shader);

    auto point_light = Entity::create("PointLight");
    auto const point_light_comp = point_light->add_component<PointLight>(PointLight::create());
    point_light_comp->diffuse = diffuse;
    light_material->color = glm::vec4(diffuse.x, diffuse.y, diffuse.z, 1.0f);
    point_light->add_component<Cube>(Cube::create(light_material));
    point_light->transform->set_parent(parent);

    return point_light;
}

std::shared_ptr<Entity> create_spot_light(glm::vec3 const& diffuse, std::shared_ptr<Transform> const& parent)
{
    auto light_shader = ShaderFactory::create("./res/shaders/light.vert", "./res/shaders/light.frag");
    auto const light_material = Material::create(light_shader);

    auto spot_light = Entity::create("SpotLight");
    auto const spot_light_comp = spot_light->add_component<SpotLight>(SpotLight::create());
    spot_light_comp->diffuse = diffuse;
    light_material->color = glm::vec4(diffuse.x, diffuse.y, diffuse.z, 1.0f);
    spot_light->add_component<Cube>(Cube::create(light_material));
    spot_light->transform->set_parent(parent);

    return spot_light;
}

std::shared_ptr<Entity> create_cube(std::string const& name, std::shared_ptr<Material> const& material)
{
    auto cube = Entity::create(name);
    cube->add_component<Cube>(Cube::create(material));

    return cube;
}

std::shared_ptr<Entity> create_cube(std::string const& name, std::string const& diffuse_texture_path, std::shared_ptr<Material> const& material)
{
    auto cube = Entity::create(name);
    cube->add_component<Cube>(Cube::create(diffuse_texture_path, material));

    return cube;
}

std::shared_ptr<Entity> create_cube(std::string const& name, std::string const& diffuse_texture_path, std::string const& specular_texture_path, std::shared_ptr<Material> const& material)
{
    auto cube = Entity::create(name);
    cube->add_component<Cube>(Cube::create(diffuse_texture_path, specular_texture_path, material));

    return cube;
}
}
