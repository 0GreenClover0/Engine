#pragma once

#include <memory>

#include "Camera.h"
#include "Window.h"

class Game
{
public:
    explicit Game(std::shared_ptr<Window> const& window);
    void initialize();

    std::shared_ptr<Window> window;

private:
    std::shared_ptr<Entity> m_camera;
    std::shared_ptr<Camera> m_camera_comp;
};
