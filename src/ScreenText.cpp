#include "ScreenText.h"

#include "AK/AK.h"
#include "Editor.h"
#include "Entity.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"

#if EDITOR
#include "imgui_stdlib.h"
#endif

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<ScreenText> ScreenText::create()
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, 2);

    auto text = std::make_shared<ScreenText>(AK::Badge<ScreenText> {}, ui_material, "Example text", glm::vec2(0, 0), 40, 0xff0099ff,
                                             FW1_RESTORESTATE | FW1_CENTER | FW1_VCENTER);

    return text;
}

std::shared_ptr<ScreenText> ScreenText::create(std::shared_ptr<Material> const& material, std::string const& content,
                                               glm::vec2 const& position, float const font_size, u32 const color, u16 const flags)
{
    auto text = std::make_shared<ScreenText>(AK::Badge<ScreenText> {}, material, content, position, font_size, color, flags);
    return text;
}

ScreenText::ScreenText(AK::Badge<ScreenText>, std::shared_ptr<Material> const& material, std::string const& content,
                       glm::vec2 const& position, float const font_size, u32 const color, u16 const flags)
    : Drawable(material), text(content), position(position), font_size(font_size), color(color),
      flags(flags | FW1_RESTORESTATE) // Restore DX11 state by default
{
}

ScreenText::~ScreenText()
{
    if (m_font_wrapper)
        m_font_wrapper->Release();

    if (m_d_write_factory)
        m_d_write_factory->Release();

    if (m_d_write_text_format)
        m_d_write_text_format->Release();

    if (m_d_write_text_layout)
        m_d_write_text_layout->Release();
}

void ScreenText::initialize()
{
    Drawable::initialize();

    if (font_name.empty())
    {
        if (!Renderer::loaded_fonts.empty())
        {
            font_name = Renderer::loaded_fonts.front().family_name;
            bold = Renderer::loaded_fonts.front().bold;
        }
    }

    refresh_layout();
}

void ScreenText::draw() const
{
    glm::vec3 const entity_pos = entity->transform->get_position();
    glm::vec2 const screen_size = {Renderer::get_instance()->screen_width, Renderer::get_instance()->screen_height};

    glm::vec2 pos_to_draw = {AK::map_range_clamped(-1.0f, 1.0f, 0, screen_size.x, entity_pos.x),
                             screen_size.y - AK::map_range_clamped(-1.0f, 1.0f, 0, screen_size.y, entity_pos.y)};

    if (m_align_to_center)
    {
        pos_to_draw = {AK::map_range_clamped(-1.0f, 3.0f, 0, screen_size.x, entity_pos.x),
                       screen_size.y - AK::map_range_clamped(-1.0f, 3.0f, 0, screen_size.y, entity_pos.y + 2.0f)};

        glm::vec2 const align_offset = {pos_to_draw.x - m_layout_width * 0.5f, pos_to_draw.y - m_layout_height * 0.5f};
        pos_to_draw += align_offset;
    }

    m_font_wrapper->DrawTextLayout(RendererDX11::get_instance_dx11()->get_device_context(), m_d_write_text_layout, pos_to_draw.x,
                                   pos_to_draw.y, color, flags);
}

#if EDITOR
void ScreenText::draw_editor()
{
    Component::draw_editor();

    std::string preview = {};
    if (Renderer::loaded_fonts.size() <= 0)
    {
        preview = "NONE";
    }
    else if (!font_name.empty())
    {
        preview = font_name;
    }
    else
    {
        preview = Renderer::loaded_fonts.front().family_name;
    }

    if (ImGui::BeginCombo("Font family", preview.c_str()))
    {
        for (auto const& font : Renderer::loaded_fonts)
        {
            bool const is_selected = font.family_name == font_name;
            if (ImGui::Selectable(font.family_name.c_str(), is_selected))
            {
                font_name = font.family_name;
                refresh_layout();
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::Checkbox("Bold", &bold))
    {
        refresh_layout();
    }

    ImGui::InputText("Text", &text);

    set_text(text);

    ImGui::Checkbox("Align to Center", &m_align_to_center);

    realign_text(m_align_to_center);
}
#endif

void ScreenText::set_text(std::string const& new_content)
{
    text = new_content;
    refresh_layout();
}

void ScreenText::realign_text(bool const center) const
{
    if (center)
    {
        HRESULT hr = m_d_write_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        assert(SUCCEEDED(hr));

        hr = m_d_write_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        assert(SUCCEEDED(hr));
    }
    else
    {
        HRESULT hr = m_d_write_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
        assert(SUCCEEDED(hr));

        hr = m_d_write_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        assert(SUCCEEDED(hr));
    }
}

D3D11_VIEWPORT ScreenText::get_viewport()
{
    UINT num_viewports = 1;
    D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    RendererDX11::get_instance_dx11()->get_device_context()->RSGetViewports(&num_viewports, viewports);
    return viewports[0];
}

void ScreenText::refresh_layout()
{
    if (m_d_write_factory != nullptr)
    {
        m_d_write_factory->Release();
        m_d_write_factory = nullptr;
    }

    if (m_d_write_text_format != nullptr)
    {
        m_d_write_text_format->Release();
        m_d_write_text_format = nullptr;
    }

    if (m_d_write_text_layout != nullptr)
    {
        m_d_write_text_layout->Release();
        m_d_write_text_layout = nullptr;
    }

    m_viewport = get_viewport();

    HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_FW1_factory);
    assert(SUCCEEDED(hr));

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_d_write_factory));
    assert(SUCCEEDED(hr));

    m_d_write_factory->CreateTextFormat(AK::string_to_wstring(font_name).c_str(), // Font family name
                                        nullptr, // Font collection (NULL for the system font collection)
                                        bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                                        DWRITE_FONT_STRETCH_NORMAL,
                                        font_size, // Font size
                                        L"pl-PL", // Locale
                                        &m_d_write_text_format);
    assert(SUCCEEDED(hr));

    std::wstring const content = AK::string_to_wstring(text);

    hr = m_d_write_factory->CreateTextLayout(content.data(), text.size(), m_d_write_text_format, 2048, 2048, &m_d_write_text_layout);
    assert(SUCCEEDED(hr));

    DWRITE_TEXT_METRICS text_metrics = {};

    hr = m_d_write_text_layout->GetMetrics(&text_metrics);
    assert(SUCCEEDED(hr));

    hr = m_d_write_factory->CreateTextLayout(content.data(), // Text to be laid out
                                             text.size(), // Length of the text
                                             m_d_write_text_format, // Text format
                                             text_metrics.width, // Use measured text width
                                             text_metrics.height, // Use measured text height
                                             &m_d_write_text_layout // Output text layout object
    );
    assert(SUCCEEDED(hr));

    hr = m_FW1_factory->CreateFontWrapper(RendererDX11::get_instance_dx11()->get_device(), AK::string_to_wstring(font_name).c_str(),
                                          &m_font_wrapper);
    assert(SUCCEEDED(hr));

    m_FW1_factory->Release();

    // Create a new text layout with the updated content
    hr = m_d_write_factory->CreateTextLayout(content.data(), text.size(), m_d_write_text_format, m_layout_width, m_layout_height,
                                             &m_d_write_text_layout);
    assert(SUCCEEDED(hr));

    realign_text(m_align_to_center);
    DWRITE_TEXT_RANGE tr = {};
    tr.length = 256;
    tr.startPosition = 0;

    realign_text(m_align_to_center);

    hr = m_d_write_text_layout->SetFontSize(font_size, tr);
    assert(SUCCEEDED(hr));
}
