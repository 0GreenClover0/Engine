#pragma once

#include "ConstantBufferTypes.h"
#include "Model.h"

#include <d3d11.h>

class Water final : public Model
{
public:
    static std::shared_ptr<Water> create();
    static std::shared_ptr<Water> create(u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material);

    explicit Water(AK::Badge<Water>, std::shared_ptr<Material> const& material);
    Water(AK::Badge<Water>, u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material);

    virtual void draw() const override;
    virtual void prepare() override;
    virtual void reprepare() override;

    virtual void draw_editor() override;

    void add_wave();
    void remove_wave(u32 const index);

    std::string texture_path = "";
    std::vector<DXWave> waves = {};

    u32 tesselation_level = 2;

private:
    void create_constant_buffer_wave();
    void set_constant_buffer() const;

    ID3D11Buffer* m_constant_buffer_wave = nullptr;
};
