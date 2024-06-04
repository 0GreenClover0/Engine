#pragma once

#include "Drawable.h"
#include "GBuffer.h"

class Particle : public Drawable
{
public:
    virtual void initialize() override;

    virtual void update() override;

    glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f};

private:
    void update_particle();

    ID3D11Buffer* m_constant_buffer_particle = nullptr;
};
