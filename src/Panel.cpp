#include "Panel.h"

#include "Renderer.h"
#include "ResourceManager.h"

#if EDITOR
#include "imgui_stdlib.h"
#endif

std::shared_ptr<Panel> Panel::create()
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, Renderer::ui_render_order + 1);
    ui_material->casts_shadows = false;

    auto panel = std::make_shared<Panel>(AK::Badge<Panel> {}, ui_material);

    panel->prepare();

    return panel;
}

Panel::Panel(AK::Badge<Panel>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Panel::awake()
{
    set_can_tick(true);
}

void Panel::draw() const
{
    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    if (m_mesh != nullptr)
    {
        m_mesh->draw();
    }

    Renderer::get_instance()->restore_default_rasterizer_draw_type();
}

#if EDITOR
void Panel::draw_editor()
{
    Drawable::draw_editor();

    ImGui::InputText("Background Path", &background_path);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        reprepare();
    }
}
#endif

void Panel::reprepare()
{
    Drawable::reprepare();
    prepare();
}

void Panel::prepare()
{
    m_mesh = create_sprite();
}

std::shared_ptr<Mesh> Panel::create_sprite() const
{
    std::vector<Vertex> const vertices = {
        {glm::vec3(-1.0f, -1.0f, 0.0f), {}, {0.0f, 0.0f}}, // bottom left
        {glm::vec3(1.0f, -1.0f, 0.0f), {}, {1.0f, 0.0f}}, // bottom right
        {glm::vec3(1.0f, 1.0f, 0.0f), {}, {1.0f, 1.0f}}, // top right
        {glm::vec3(-1.0f, 1.0f, 0.0f), {}, {0.0f, 1.0f}}, // top left
    };

    std::vector<u32> const indices = {0, 1, 2, 0, 2, 3};

    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};
    TextureSettings texture_settings = {};
    texture_settings.wrap_mode_x = TextureWrapMode::ClampToEdge;
    texture_settings.wrap_mode_y = TextureWrapMode::ClampToEdge;

    if (!background_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(background_path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(0, background_path, vertices, indices, textures, DrawType::Triangles, material);
}
