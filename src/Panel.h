#pragma once

#include "Drawable.h"
#include "Mesh.h"

class Panel : public Drawable
{
public:
    static std::shared_ptr<Panel> create();
    explicit Panel(AK::Badge<Panel>, std::shared_ptr<Material> const& material);

    virtual void awake() override;

    virtual void draw() const override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void prepare();
    virtual void reprepare() override;

    std::string background_path = "./res/textures/white.jpg";

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;

    std::shared_ptr<Mesh> m_mesh = {};
};
