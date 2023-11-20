#include "PlanetarySystem.h"

#include <format>
#include <glm/gtc/random.hpp>

#include "Ellipse.h"
#include "Entity.h"
#include "Material.h"
#include "Sphere.h"

PlanetarySystem::PlanetarySystem() = default;

void PlanetarySystem::awake()
{
    auto standard_shader = std::make_shared<Shader>("./res/shaders/vertex.vert", "./res/shaders/fragment.frag");
    auto standard_material = std::make_shared<Material>(standard_shader);

    auto const sun = Entity::create("Sun");
    sun->transform->set_parent(entity->transform);

    {
        sun->add_component<Model>("./res/models/sun/13913_Sun_v2_l3.obj", standard_material);
        sun->transform->set_local_scale(glm::vec3(0.002f, 0.002f, 0.002f));
    }

    auto const planets_parent = Entity::create("PlanetsParent");

    {
        planets_parent->transform->set_parent(sun->transform);
        planets_parent->transform->set_local_scale(glm::vec3(500.0f, 500.0f, 500.0f));
    }

    {
        auto sun_comp = sun->add_component<AstronomicalObject>();
        auto custom_sphere_shader = std::make_shared<Shader>("./res/shaders/vertex.vert", "./res/shaders/fragment.frag");//std::make_shared<Shader>("./vertex_sphere.vert", "./fragment_sphere.frag", "./geometry.geo");
        auto custom_sphere_material = std::make_shared<Material>(custom_sphere_shader);

        glm::vec2 orbit = glm::vec2(2.5f, 2.5f);
        float planet_scale = 0.04f;
        for (uint32_t i = 1; i <= planet_count; ++i)
        {
            auto const planet = Entity::create(std::format("Planet{}", i));
            auto planet_comp = planet->add_component<AstronomicalObject>();

            custom_sphere_material->color = glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));

            auto const sphere = planet->add_component<Sphere>(3.0f, default_sphere_sector_and_stack_count, default_sphere_sector_and_stack_count, "./res/textures/stone.jpg", custom_sphere_material);

            planet_comp->alpha = glm::linearRand(0.0f, 360.0f);
            planet_comp->orbit = glm::vec2(orbit);
            planet_comp->rotation_speed = glm::linearRand(5.0f, 25.0f);
            planet_comp->model = sphere;

            planets_parent->add_component<class Ellipse>(0.0f, 0.0f, orbit.x, orbit.y, 60, custom_sphere_material);

            planet_comp->speed = i == 1 ? 2.0f : planets.back().lock()->speed * 0.6f;

            orbit.x += 0.9f;
            orbit.y += 0.65f;

            planet->transform->set_parent(planets_parent->transform);
            planet->transform->set_local_position(glm::vec3(i, 0.0f, i));
            planet->transform->set_local_scale(glm::vec3(planet_scale, planet_scale, planet_scale));
            planet->transform->set_euler_angles(glm::vec3(
                glm::linearRand(0.0f, 100.0f),
                planet->transform->get_euler_angles().y,
                planet->transform->get_euler_angles().z
            ));
            planets.emplace_back(planet_comp);

            planet_scale += 0.02f;
        }

        for (uint32_t i = 1; i <= moon_count; ++i)
        {
            auto const moon = Entity::create(std::format("Moon{}", i));
            auto moon_comp = moon->add_component<AstronomicalObject>();

            float const color = glm::linearRand(0.0f, 1.0f);
            custom_sphere_material->color = glm::vec3(color, color, color);

            auto const sphere = moon->add_component<Sphere>(1.0f, default_sphere_sector_and_stack_count, default_sphere_sector_and_stack_count, "./res/textures/stone.jpg", custom_sphere_material);

            moon_comp->alpha = glm::linearRand(0.0f, 360.0f);
            moon_comp->orbit = glm::vec2(glm::linearRand(3.5f, 4.5f), glm::linearRand(3.5f, 4.5f));
            moon_comp->rotation_speed = glm::linearRand(5.0f, 25.0f);
            moon_comp->speed = i == 1 ? 2.0f : moons.back().lock()->speed * 0.85f;
            moon_comp->model = sphere;

            planets[planets.size() - i].lock()->entity->add_component<class Ellipse>(
                0.0f,
                0.0f,
                moon_comp->orbit.x,
                moon_comp->orbit.y,
                40,
                custom_sphere_material
            );

            moon->transform->set_parent(planets[planets.size() - i].lock()->entity->transform);
            moon->transform->set_local_position(glm::vec3(i, 0.0f, i));
            float const moon_scale = glm::linearRand(0.5f, 1.0f);
            moon->transform->set_local_scale(glm::vec3(moon_scale, moon_scale, moon_scale));
            moon->transform->set_euler_angles(glm::vec3(
                glm::linearRand(0.0f, 100.0f),
                moon->transform->get_euler_angles().y,
                moon->transform->get_euler_angles().z
            ));
            moons.emplace_back(moon_comp);
        }
    }
}

void PlanetarySystem::change_detail(float const detail) const
{
    for (auto const& planet : planets)
    {
        std::shared_ptr<Sphere> const model = std::static_pointer_cast<Sphere>(planet.lock()->model.lock());
        model->stack_count = default_sphere_sector_and_stack_count * detail;
        model->sector_count = default_sphere_sector_and_stack_count * detail;
        model->reprepare();
    }

    for (auto const& moon : moons)
    {
        std::shared_ptr<Sphere> const model = std::static_pointer_cast<Sphere>(moon.lock()->model.lock());
        model->stack_count = default_sphere_sector_and_stack_count * detail;
        model->sector_count = default_sphere_sector_and_stack_count * detail;
        model->reprepare();
    }
}
