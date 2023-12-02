#pragma once

#include "Component.h"
#include "Drawable.h"
#include "MainScene.h"
#include "Transform.h"

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    Entity(std::string name);
    static std::shared_ptr<Entity> create(std::string name = "Entity");
    static std::shared_ptr<Entity> create(std::string const& guid, std::string const& name);

    template <class T>
    std::shared_ptr<T> add_component()
    {
        auto component = std::make_shared<T>();
        components.emplace_back(component);
        component->entity = shared_from_this();
        component->initialize();

        // TODO: Assumption that this entity belongs to the main scene
        if (auto const& scene = MainScene::get_instance(); scene->is_after_start)
        {
            // TODO: Order of component Awake, Start on components instantiated inside the Awake call below is wrong.
            //         The new component's Awake and Start will finish before the Start of this component.
            component->awake();
            component->start();
        }

        return component;
    }

    template <class T, typename... TArgs>
    std::shared_ptr<T> add_component(TArgs&&... args)
    {
        auto component = std::make_shared<T>(std::forward<TArgs>(args)...);
        components.emplace_back(component);
        component->entity = shared_from_this();
        component->initialize();

        // TODO: Assumption that this entity belongs to the main scene
        if (auto const& scene = MainScene::get_instance(); scene->is_after_start)
        {
            component->awake();
            component->start();
        }

        return component;
    }

    template<typename T>
    std::shared_ptr<T> get_component()
    {
        for (auto const& component : components)
        {
            auto comp = std::dynamic_pointer_cast<T>(component);
            if (comp != nullptr)
                return comp;
        }

        return nullptr;
    }

    std::string name;
    std::string guid;
    size_t hashed_guid;
    std::shared_ptr<Transform> transform;
    std::vector<std::shared_ptr<Component>> components = {};
    std::vector<std::shared_ptr<Drawable>> drawables = {};

private:

    std::string parent_guid; // NOTE: Only for serialization

    friend class SceneSerializer;
};
