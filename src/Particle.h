#pragma once

#include "Drawable.h"
#include "GBuffer.h"

class Particle final : public Component
{
public:
    static std::shared_ptr<Particle> create();
    explicit Particle(AK::Badge<Particle>);

    virtual void initialize() override;

    virtual void update() override;

    virtual void draw_editor() override;

    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

private:
    void update_particle();

    ID3D11Buffer* m_constant_buffer_particle = nullptr;
};
