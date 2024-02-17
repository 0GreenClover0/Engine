#pragma once

#include "Component.h"
#include "Drawable.h"
#include "MainScene.h"
#include "Transform.h"

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    explicit Entity(std::string name);
    static std::shared_ptr<Entity> create(std::string const& name = "Entity");
    static std::shared_ptr<Entity> create(std::string const& guid, std::string const& name);

    template <class T>
    std::shared_ptr<T> add_component()
    {
        auto component = std::make_shared<T>();
        components.emplace_back(component);
        component->entity = shared_from_this();

        // Initialization for internal components
        component->initialize();

        // TODO: Assumption that this entity belongs to the main scene
        // NOTE: Currently we treat manually assigning references in the Game initialization code as if someone would ex. drag a reference
        //       to an object in the Unity's inspector. This means that the Awake call of the constructed components should be called
        //       after all of these references were assigned, or more precisely, when the game has started. We do this manually, by
        //       gathering all the components in the components_to_awake vector and calling Awake on those when the game starts.
        //       If the component is constructed during the gameplay, we call the Awake immediately here.
        if (MainScene::get_instance()->is_running)
        {
            component->awake();
        }
        else
        {
            MainScene::get_instance()->components_to_awake.emplace_back(component);
        }

        MainScene::get_instance()->components_to_start.emplace_back(component);

        return component;
    }

    template <class T>
    std::shared_ptr<T> add_component(std::shared_ptr<T> component)
    {
        components.emplace_back(component);
        component->entity = shared_from_this();

        // Initialization for internal components
        component->initialize();

        // TODO: Assumption that this entity belongs to the main scene
        // NOTE: Currently we treat manually assigning references in the Game initialization code as if someone would ex. drag a reference
        //       to an object in the Unity's inspector. This means that the Awake call of the constructed components should be called
        //       after all of these references were assigned, or more precisely, when the game has started. We do this manually, by
        //       gathering all the components in the components_to_awake vector and calling Awake on those when the game starts.
        //       If the component is constructed during the gameplay, we call the Awake immediately here.
        if (MainScene::get_instance()->is_running)
        {
            component->awake();
        }
        else
        {
            MainScene::get_instance()->components_to_awake.emplace_back(component);
        }

        MainScene::get_instance()->components_to_start.emplace_back(component);

        return component;
    }

    template <class T, typename... TArgs>
    std::shared_ptr<T> add_component(TArgs&&... args)
    {
        auto component = std::make_shared<T>(std::forward<TArgs>(args)...);
        components.emplace_back(component);
        component->entity = shared_from_this();

        // Initialization for internal components
        component->initialize();

        // TODO: Assumption that this entity belongs to the main scene
        // NOTE: Currently we treat manually assigning references in the Game initialization code as if someone would ex. drag a reference
        //       to an object in the Unity's inspector. This means that the Awake call of the constructed components should be called
        //       after all of these references were assigned, or more precisely, when the game has started. We do this manually, by
        //       gathering all the components in the components_to_awake vector and calling Awake on those when the game starts.
        //       If the component is constructed during the gameplay, we call the Awake immediately here.
        if (MainScene::get_instance()->is_running)
        {
            component->awake();
        }
        else
        {
            MainScene::get_instance()->components_to_awake.emplace_back(component);
        }

        MainScene::get_instance()->components_to_start.emplace_back(component);

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
