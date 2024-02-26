#pragma once

#include "Scene.h"

class MainScene : public Scene
{
public:
    static void set_instance(std::shared_ptr<Scene> const& scene)
    {
        m_instance = scene;
    }

    static std::shared_ptr<Scene> get_instance()
    {
        return m_instance;
    }

    MainScene(MainScene const&) = delete;
    void operator=(MainScene const&) = delete;

private:
    inline static std::shared_ptr<Scene> m_instance;
};
