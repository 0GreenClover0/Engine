#include "Water.h"
#include "ConstantBufferTypes.h"
#include "MeshFactory.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "Skybox.h"
#include "Texture.h"
#include "TextureLoader.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_stdlib.h>

std::shared_ptr<Water> Water::create()
{
    auto const shader = ResourceManager::get_instance().load_shader("./res/shaders/water.hlsl", "./res/shaders/water.hlsl");
    auto material = Material::create(shader);
    material->casts_shadows = false;
    material->needs_skybox = true;
    material->needs_forward_rendering = true;
    auto water = std::make_shared<Water>(AK::Badge<Water> {}, material);
    water->tesselation_level = 5;

    return water;
}

std::shared_ptr<Water> Water::create(u32 tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material)
{
    auto water = std::make_shared<Water>(AK::Badge<Water> {}, tesselation_level, texture_path, material);
    water->add_wave();
    return water;
}

Water::Water(AK::Badge<Water>, std::shared_ptr<Material> const& material) : Model(material)
{
    create_constant_buffer_wave();
    texture_path = "res/textures/water.jpg";

    Water::prepare();
}

Water::Water(AK::Badge<Water>, u32 const tesselation_level, std::string const& texture_path, std::shared_ptr<Material> const& material)
    : Model(material), texture_path(texture_path), tesselation_level(tesselation_level)
{
    create_constant_buffer_wave();

    Water::prepare();
}

void Water::draw() const
{
    set_constant_buffer();

    Skybox::get_instance()->bind();

    Model::draw();

    Skybox::get_instance()->unbind();
}

void Water::prepare()
{
    i32 const num_triangles = 2 << (2 * tesselation_level);
    i32 const num_vertices = (1 << (tesselation_level) + 1) * (1 << (tesselation_level) + 1);

    std::vector<Vertex> vertices = {};
    vertices.resize(num_vertices);

    std::vector<u32> indices = {};
    indices.resize(num_triangles * 3);

    float constexpr size = 50.0f;
    float const step = size / (1 << tesselation_level);
    float const texture_step = 1.0f / (1 << tesselation_level);

    i32 index = 0;
    for (i32 i = 0; i <= (1 << tesselation_level); ++i)
    {
        for (i32 j = 0; j <= (1 << tesselation_level); ++j)
        {
            vertices[index].position = {j * step - size / 2.0f, 0.0f, i * step - size / 2.0f};
            vertices[index].normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertices[index].texture_coordinates = glm::vec2(j * texture_step - 0.5f, i * texture_step - 0.5f);
            index++;
        }
    }

    i32 triangle_index = 0;
    for (i32 i = 0; i < (1 << tesselation_level); i++)
    {
        for (i32 j = 0; j < (1 << tesselation_level); j++)
        {
            // Indices of the four vertices of the current square
            i32 const top_left = i * ((1 << tesselation_level) + 1) + j;
            i32 const top_right = top_left + 1;
            i32 const bottom_left = (i + 1) * ((1 << tesselation_level) + 1) + j;
            i32 const bottom_right = bottom_left + 1;

            // First triangle
            indices[triangle_index++] = top_left;
            indices[triangle_index++] = bottom_left;
            indices[triangle_index++] = top_right;

            // Second triangle
            indices[triangle_index++] = top_right;
            indices[triangle_index++] = bottom_left;
            indices[triangle_index++] = bottom_right;
        }
    }

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};

    if (!texture_path.empty())
    {
        diffuse_maps = {ResourceManager::get_instance().load_texture(texture_path, TextureType::Diffuse)};
    }

    m_meshes.push_back(
        ResourceManager::get_instance().load_mesh(m_meshes.size(), "WATER", vertices, indices, diffuse_maps, m_draw_type, material));
}

void Water::reprepare()
{
    m_meshes.clear();

    prepare();
}

void Water::draw_editor()
{
    Drawable::draw_editor();

    u32 constexpr min = 0;
    u32 constexpr max = 9;

    if (ImGui::SliderScalar("Value", ImGuiDataType_U32, &tesselation_level, &min, &max, "%u"))
    {
        reprepare();
    }

    // List existing waves
    for (size_t i = 0; i < waves.size(); ++i)
    {
        if (ImGui::TreeNode(("Wave " + std::to_string(i)).c_str()))
        {
            ImGui::DragFloat2("Direction", &waves[i].direction[0]);
            ImGui::DragFloat("Speed", &waves[i].speed);
            ImGui::DragFloat("Steepness", &waves[i].steepness);
            ImGui::DragFloat("Wave Length", &waves[i].wave_length);
            ImGui::DragFloat("Amplitude", &waves[i].amplitude);

            if (ImGui::Button(("Delete##" + std::to_string(i)).c_str()))
            {
                remove_wave(i);
                --i; // Adjust index after erasing element
            }

            ImGui::TreePop();
        }
    }

    if (ImGui::Button("Add New Wave"))
    {
        add_wave();
    }

    ImGui::InputText("Texture Path", &texture_path);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        reprepare();
    }
}

void Water::add_wave()
{
    if (waves.size() >= 15)
        return;

    DXWave wave = {};
    wave.direction = glm::vec2(1.0f, 0.5f);
    wave.amplitude = 0.5f;
    wave.speed = 100.0f;
    wave.steepness = 0.9f;
    wave.wave_length = 500.0f;
    waves.emplace_back(wave);
}

void Water::remove_wave(u32 const index)
{
    if (index > waves.size() - 1)
        return;

    waves.erase(waves.begin() + index);
}

/*
* This function calculates the height of the water at a given position
* It's not precise, but approximated
* position is in 2D space, meaning x and z coordinates of 3D space
* use AK::convert_3d_to_2d when passing position (recommended)
*/
float Water::get_wave_height(glm::vec2 const& position) const
{
    float const gravity = 9.8f;
    float constexpr PI = 3.14159265359f;
    // Tweaking this will make the calculations more or less accurate
    // But will obviously affect performance
    u32 constexpr iterations = 5;
    float height = 0.0f;
    float time = glfwGetTime();
    for (auto const& wave : waves)
    {
        float frequency = sqrt(gravity * 2.0f * PI / wave.wave_length);
        float steepness = wave.steepness / (frequency * wave.amplitude * waves.size());
        float phi = wave.speed * 2.0f / wave.wave_length;
        glm::vec2 new_position = position;
        float per_wave_height = 0.0f;
        for (i32 i = 0; i < iterations; i++)
        {
            glm::vec2 offset = {};
            offset.x = steepness * wave.amplitude * wave.direction.x * cos(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
            offset.y = steepness * wave.amplitude * wave.direction.x * cos(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
            per_wave_height = wave.amplitude * sin(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
            new_position = position - offset;
        }
        height += per_wave_height;
    }

    return height;
}

void Water::create_constant_buffer_wave()
{
    auto const renderer = RendererDX11::get_instance_dx11();

    D3D11_BUFFER_DESC wave_buffer_desc = {};
    wave_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    wave_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    wave_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    wave_buffer_desc.MiscFlags = 0;
    wave_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(ConstantBufferWave) + (16 - (sizeof(ConstantBufferWave) % 16)));

    HRESULT const hr = renderer->get_device()->CreateBuffer(&wave_buffer_desc, nullptr, &m_constant_buffer_wave);
    assert(SUCCEEDED(hr));
}

void Water::set_constant_buffer() const
{
    auto const renderer = RendererDX11::get_instance_dx11();

    ConstantBufferWave wave_buffer = {};
    wave_buffer.time = static_cast<float>(glfwGetTime());
    wave_buffer.number_of_waves = waves.size();

    for (u32 i = 0; i < waves.size(); i++)
    {
        wave_buffer.waves[i] = waves[i];
    }

    D3D11_MAPPED_SUBRESOURCE wave_buffer_resource = {};

    HRESULT const hr = renderer->get_device_context()->Map(m_constant_buffer_wave, 0, D3D11_MAP_WRITE_DISCARD, 0, &wave_buffer_resource);
    assert(SUCCEEDED(hr));

    CopyMemory(wave_buffer_resource.pData, &wave_buffer, sizeof(ConstantBufferWave));

    renderer->get_device_context()->Unmap(m_constant_buffer_wave, 0);
    renderer->get_device_context()->VSSetConstantBuffers(1, 1, &m_constant_buffer_wave);
}
