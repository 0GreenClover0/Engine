#pragma once

#include <memory>

#include "Shader.h"

class Material
{
public:
    explicit Material(std::shared_ptr<Shader> shader);

    std::shared_ptr<Shader> shader;

private:
    friend class SceneSerializer;
};
