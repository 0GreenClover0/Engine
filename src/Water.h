#pragma once
#include "Model.h"
#include "ConstantBufferTypes.h"
#include "d3d11.h"

class Water final : public Model
{
public:
	static std::shared_ptr<Water> create();
	static std::shared_ptr<Water> create(u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material);

	explicit Water(AK::Badge<Water>, std::shared_ptr<Material> const& material);
	Water(AK::Badge<Water>, u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material);
	
	virtual std::string get_name() const override;

	virtual void draw() const override;
	virtual void prepare() override;

	virtual void draw_editor() override;

	void add_wave();
	void remove_wave(u32 index);

private:
	// could be u32 but ImGui doesn't support unsigned types
	i32 m_tesselation_level = 6;
	std::string m_texture_path;
	std::vector<std::shared_ptr<DXWave>> waves;
	ID3D11Buffer* m_constant_buffer_wave = nullptr;

	friend class SceneSerializer;
};

