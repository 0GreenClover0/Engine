#include "Water.h"

#include "ConstantBufferTypes.h"
#include "MeshFactory.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "Skybox.h"
#include "Texture.h"
#include "TextureLoader.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

std::shared_ptr<Water> Water::create()
{
    auto const shader = ResourceManager::get_instance().load_shader("./res/shaders/water.hlsl", "./res/shaders/water.hlsl");
    auto material = Material::create(shader, 1001);
    material->casts_shadows = false;
    material->needs_skybox = true;
    material->needs_forward_rendering = true;
    auto water = std::make_shared<Water>(AK::Badge<Water> {}, material);
    water->add_wave();

    water->m_ps_buffer.top_color = glm::vec4(0.1f, 0.1f, 0.5f, 1.0f);
    water->m_ps_buffer.bottom_color = glm::vec4(0.0f, 0.0f, 0.4f, 1.0f);
    water->m_ps_buffer.normalmap_scroll_speed_0 = 1.0f / 500.0f;
    water->m_ps_buffer.normalmap_scroll_speed_1 = 1.0f / 500.0f * 1.3f;
    water->m_ps_buffer.normalmap_scale0 = 35.0f;
    water->m_ps_buffer.normalmap_scale1 = 10.0f;
    water->m_ps_buffer.phong_contribution = 0.98f;
    water->m_ps_buffer.combined_amplitude = 0.0f;
    // This is not right, but commit on other branch totally changes how Water is created
    // so it doesn't matter, let it be like that for now
    for (int i = 0; i < water->waves.size(); i++)
    {
        water->m_ps_buffer.combined_amplitude += water->waves[i].amplitude;
    }
    return water;
}

std::shared_ptr<Water> Water::create(u32 tesselation_level, std::shared_ptr<Material> const& material)
{
    auto water = std::make_shared<Water>(AK::Badge<Water> {}, tesselation_level, material);
    water->add_wave();
    return water;
}

Water::Water(AK::Badge<Water>, std::shared_ptr<Material> const& material) : Model(material)
{
    create_constant_buffer_wave();
    m_normal_map0 = ResourceManager::get_instance().load_texture("./res/textures/water/water_normal1.png", TextureType::Diffuse);
    m_normal_map1 = ResourceManager::get_instance().load_texture("./res/textures/water/water_normal2.png", TextureType::Diffuse);
    Water::prepare();
}

Water::Water(AK::Badge<Water>, u32 const tesselation_level, std::shared_ptr<Material> const& material)
    : Model(material), tesselation_level(tesselation_level)
{
    create_constant_buffer_wave();

    Water::prepare();
}

void Water::draw() const
{
    auto const renderer = RendererDX11::get_instance_dx11();
    renderer->get_device_context()->PSSetShaderResources(18, 1, &m_normal_map0->shader_resource_view);
    renderer->get_device_context()->PSSetShaderResources(19, 1, &m_normal_map1->shader_resource_view);
    set_constant_buffer();

    Skybox::get_instance()->bind();

    Model::draw();

    Skybox::get_instance()->unbind();
    ID3D11ShaderResourceView* null_shader_resource_view = nullptr;
    renderer->get_device_context()->PSSetShaderResources(18, 1, &null_shader_resource_view);
    renderer->get_device_context()->PSSetShaderResources(19, 1, &null_shader_resource_view);
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

    m_meshes.push_back(
        ResourceManager::get_instance().load_mesh(m_meshes.size(), "WATER", vertices, indices, diffuse_maps, m_draw_type, material));
}

void Water::reprepare()
{
    m_meshes.clear();

    prepare();
}

#if EDITOR
void Water::draw_editor()
{
    Drawable::draw_editor();

    u32 constexpr min = 0;
    u32 constexpr max = 9;

    if (ImGui::SliderScalar("Tesselation", ImGuiDataType_U32, &tesselation_level, &min, &max, "%u"))
    {
        reprepare();
    }

    // Pixel Shader Constants
    ImGui::ColorEdit3("Top Color", &m_ps_buffer.top_color[0]);
    ImGui::ColorEdit3("Bottom Color", &m_ps_buffer.bottom_color[0]);
    ImGui::DragFloat("Normalmap Scroll Speed 0", &m_ps_buffer.normalmap_scroll_speed_0, 0.001f, -1.0f, 1.0f);
    ImGui::DragFloat("Normalmap Scroll Speed 1", &m_ps_buffer.normalmap_scroll_speed_1, 0.001f, -1.0f, 1.0f);
    ImGui::SliderFloat("Normalmap Scale 0", &m_ps_buffer.normalmap_scale0, 1.0f, 100.0f);
    ImGui::SliderFloat("Normalmap Scale 1", &m_ps_buffer.normalmap_scale1, 1.0f, 100.0f);
    ImGui::SliderFloat("Phong Contribution", &m_ps_buffer.phong_contribution, 0.0f, 1.0f);
    m_ps_buffer.combined_amplitude = 0.0f;
    for (int i = 0; i < waves.size(); i++)
    {
        m_ps_buffer.combined_amplitude += waves[i].amplitude;
    }
    // List existing waves
    for (size_t i = 0; i < waves.size(); ++i)
    {
        if (ImGui::TreeNode(("Wave " + std::to_string(i)).c_str()))
        {
            ImGui::SliderFloat2("Direction", &waves[i].direction[0], -1.0f, 1.0f);
            ImGui::DragFloat("Speed", &waves[i].speed, 0.5f, 0.0f, 1000.0f);
            ImGui::SliderFloat("Steepness", &waves[i].steepness, 0.0f, 1.0f);
            ImGui::DragFloat("Wave Length", &waves[i].wave_length, 0.5f, 0.5f, 1000.0f);
            ImGui::SliderFloat("Amplitude", &waves[i].amplitude, 0.0f, 0.1f);

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

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        reprepare();
    }
}
#endif

void Water::add_wave()
{
    if (waves.size() >= 15)
        return;

    DXWave wave = {};
    if (waves.size() != 0)
    {
        wave.direction = waves[waves.size() - 1].direction;
        wave.amplitude = waves[waves.size() - 1].amplitude;
        wave.speed = waves[waves.size() - 1].speed;
        wave.steepness = waves[waves.size() - 1].steepness;
        wave.wave_length = waves[waves.size() - 1].wave_length;
    }
    else
    {
        wave.direction = glm::vec2(1.0f, 0.5f);
        wave.amplitude = 0.5f;
        wave.speed = 100.0f;
        wave.steepness = 0.9f;
        wave.wave_length = 500.0f;
    }
    waves.emplace_back(wave);

    m_ps_buffer.combined_amplitude = 0.0f;
    for (int i = 0; i < waves.size(); i++)
    {
        m_ps_buffer.combined_amplitude += waves[i].amplitude;
    }
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
            offset.x = steepness * wave.amplitude * wave.direction.x
                     * cos(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
            offset.y = steepness * wave.amplitude * wave.direction.x
                     * cos(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
            per_wave_height =
                wave.amplitude * sin(glm::dot((frequency * wave.direction), glm::vec2(new_position.x, new_position.y)) + phi * time);
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

    HRESULT hr = renderer->get_device()->CreateBuffer(&wave_buffer_desc, nullptr, &m_constant_buffer_wave);
    assert(SUCCEEDED(hr));

    D3D11_BUFFER_DESC water_buffer_desc = wave_buffer_desc;
    water_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(ConstantBufferWater) + (16 - (sizeof(ConstantBufferWater) % 16)));

    hr = renderer->get_device()->CreateBuffer(&water_buffer_desc, nullptr, &m_constant_buffer_water);
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

    HRESULT hr = renderer->get_device_context()->Map(m_constant_buffer_wave, 0, D3D11_MAP_WRITE_DISCARD, 0, &wave_buffer_resource);
    assert(SUCCEEDED(hr));

    CopyMemory(wave_buffer_resource.pData, &wave_buffer, sizeof(ConstantBufferWave));

    renderer->get_device_context()->Unmap(m_constant_buffer_wave, 0);
    renderer->get_device_context()->VSSetConstantBuffers(1, 1, &m_constant_buffer_wave);

    D3D11_MAPPED_SUBRESOURCE water_buffer_resource = {};
    hr = renderer->get_device_context()->Map(m_constant_buffer_water, 0, D3D11_MAP_WRITE_DISCARD, 0, &water_buffer_resource);
    assert(SUCCEEDED(hr));
    CopyMemory(water_buffer_resource.pData, &m_ps_buffer, sizeof(ConstantBufferWater));
    renderer->get_device_context()->Unmap(m_constant_buffer_water, 0);
    renderer->get_device_context()->PSSetConstantBuffers(4, 1, &m_constant_buffer_water);
}
