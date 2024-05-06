#include "Water.h"
#include "Texture.h"
#include "MeshFactory.h"
#include "TextureLoader.h"
#include "Globals.h"
#include <imgui.h>
#include <ConstantBufferTypes.h>
#include <GLFW/glfw3.h>
#include <RendererDX11.h>

std::shared_ptr<Water> Water::create()
{
    auto plane = std::make_shared<Water>(AK::Badge<Water> {}, default_material);

    return plane;
}

std::shared_ptr<Water> Water::create(u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material)
{
    auto water = std::make_shared<Water>(AK::Badge<Water> {}, tesselation_level, texture_path, material);
    water->add_wave();
    return water;
}

Water::Water(AK::Badge<Water>, std::shared_ptr<Material> const& material) : Model(material)
{
    Water::prepare();
}

Water::Water(AK::Badge<Water>, u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material)
: Model(material), m_tesselation_level(tesselation_level), m_texture_path(texture_path)
{
    auto renderer = RendererDX11::get_instance_dx11();
    HRESULT hr;
    D3D11_BUFFER_DESC wave_buffer_desc = {};
    wave_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    wave_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    wave_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    wave_buffer_desc.MiscFlags = 0;
    wave_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(ConstantBufferWave) + (16 - (sizeof(ConstantBufferWave) % 16)));

    hr = renderer->get_device()->CreateBuffer(&wave_buffer_desc, nullptr, &m_constant_buffer_wave);
    Water::prepare();
}

std::string Water::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Water::draw() const
{
    HRESULT hr;
    auto renderer = RendererDX11::get_instance_dx11();
    ConstantBufferWave wave_buffer = {};
    wave_buffer.time = (float)glfwGetTime();
    for (int i = 0; i < waves.size(); i++)
    {
        wave_buffer.waves[i] = *waves[i];
    }
    wave_buffer.number_of_waves = waves.size();
    D3D11_MAPPED_SUBRESOURCE wave_buffer_resource;
    hr = renderer->get_device_context()->Map(m_constant_buffer_wave, 0, D3D11_MAP_WRITE_DISCARD, 0, &wave_buffer_resource);
    assert(SUCCEEDED(hr));

    CopyMemory(wave_buffer_resource.pData, &wave_buffer, sizeof(ConstantBufferWave));

    renderer->get_device_context()->Unmap(m_constant_buffer_wave, 0);
    renderer->get_device_context()->VSSetConstantBuffers(1, 1, &m_constant_buffer_wave);
    Model::draw();
}

void Water::prepare()
{
	int num_triangles = 2 << (2 * m_tesselation_level);
	int num_vertices = (1 << (m_tesselation_level) + 1) * (1 << (m_tesselation_level) + 1);

	std::vector<Vertex> vertices(num_vertices);
	std::vector<u32> indices(num_triangles * 3);
	
    float size = 50.0f;
	float step = size / (1 << m_tesselation_level);
	float texture_step = 1.0f / (1 << m_tesselation_level);
	int idx = 0;
	for (int i = 0; i <= (1 << m_tesselation_level); ++i)
	{
		for (int j = 0; j <= (1 << m_tesselation_level); ++j)
		{
			vertices[idx].position = { j * step - size/2.0f, 0.0f, i * step - size / 2.0f };
            vertices[idx].normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertices[idx].texture_coordinates = glm::vec2(j * texture_step - 0.5f, i * texture_step - 0.5f);
            idx++;
		}
	}

    int triangle_idx = 0;
    for (int i = 0; i < (1 << m_tesselation_level); i++)
    {
        for (int j = 0; j < (1 << m_tesselation_level); j++)
        {
            // Indices of the four vertices of the current square
            int top_left = i * ((1 << m_tesselation_level) + 1) + j;
            int top_right = top_left + 1;
            int bottom_left = (i + 1) * ((1 << m_tesselation_level) + 1) + j;
            int bottom_right = bottom_left + 1;

            // First triangle
            indices[triangle_idx++] = top_left;
            indices[triangle_idx++] = bottom_left;
            indices[triangle_idx++] = top_right;

            // Second triangle
            indices[triangle_idx++] = top_right;
            indices[triangle_idx++] = bottom_left;
            indices[triangle_idx++] = bottom_right;
        }
    }
    std::vector<Texture> diffuse_maps = { TextureLoader::get_instance()->load_texture(m_texture_path, TextureType::Diffuse)};
    m_meshes.clear();
    m_meshes.push_back(MeshFactory::create(vertices, indices, diffuse_maps, m_draw_type, m_material));
}

void Water::draw_editor()
{
    Drawable::draw_editor();
    
    if (ImGui::SliderInt("Value", &m_tesselation_level, 0, 9, "%u"))
    {
        prepare();
    }

    // List existing waves
    for (size_t i = 0; i < waves.size(); ++i)
    {
        if (ImGui::TreeNode(("Wave " + std::to_string(i)).c_str()))
        {
            ImGui::DragFloat2("Direction", &waves[i]->direction[0]);
            ImGui::DragFloat("Speed", &waves[i]->speed);
            ImGui::DragFloat("Steepness", &waves[i]->steepness);
            ImGui::DragFloat("Wave Length", &waves[i]->wave_length);
            ImGui::DragFloat("Amplitude", &waves[i]->amplitude);

            if (ImGui::Button(("Delete##" + std::to_string(i)).c_str()))
            {
                waves.erase(waves.begin() + i);
                --i; // Adjust index after erasing element
            }

            ImGui::TreePop();
        }
    }

    // Add new wave
    if (ImGui::Button("Add New Wave"))
    {
        add_wave();
    }

}
void Water::add_wave()
{
    if (waves.size() >= 15) return;
    std::shared_ptr<DXWave> wave = std::make_shared<DXWave>();
    wave->direction = glm::vec2(1.0f, 0.5f);
    wave->amplitude = 0.5f;
    wave->speed = 100.0f;
    wave->steepness = 0.9f;
    wave->wave_length = 500.0f;
    waves.push_back(wave);
}

void Water::remove_wave(u32 index)
{
    if (index > waves.size() - 1) return;
    waves.erase(waves.begin()+index);
}
