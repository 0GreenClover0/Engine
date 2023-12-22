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
    std::shared_ptr<Entity> camera;
    std::shared_ptr<Camera> camera_comp;
};
