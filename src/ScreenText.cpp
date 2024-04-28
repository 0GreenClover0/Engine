#include "ScreenText.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include <glm/gtc/type_ptr.inl>

std::shared_ptr<ScreenText> ScreenText::create()
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader);
    auto text = std::make_shared<ScreenText>(AK::Badge<ScreenText> {}, ui_material);

    return text;
}

std::shared_ptr<ScreenText> ScreenText::create(std::wstring const &content, glm::vec2 const &position,
    float const font_size, u32 const color, u16 const flags)
{
    auto text = std::make_shared<ScreenText>(AK::Badge<ScreenText> {}, content, position, font_size, color, flags);

    return text;
}

ScreenText::ScreenText(AK::Badge<ScreenText>, std::shared_ptr<Material> const& material) : Drawable(material),
    flags(FW1_RESTORESTATE)
{
}

ScreenText::ScreenText(AK::Badge<ScreenText>, std::wstring const& content, glm::vec2 const& position, float const font_size, u32 const color, u16 const flags)
    : Drawable(nullptr), text(content), position(position), font_size(font_size), color(color),
      flags(flags | FW1_RESTORESTATE) // Restore DX11 state by default
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader);
    material = ui_material;
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

    m_viewport = get_viewport();

    HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_FW1_factory);
    assert(SUCCEEDED(hr));

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_d_write_factory));
    assert(SUCCEEDED(hr));

    hr = m_d_write_factory->CreateTextFormat(
        L"Arial",                        // Font family name
        nullptr,                         // Font collection (NULL for the system font collection)
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font_size,                     // Font size
        L"en-US",                        // Locale
        &m_d_write_text_format
    );
    assert(SUCCEEDED(hr));

    hr = m_d_write_factory->CreateTextLayout(text.data(), text.size(), m_d_write_text_format, 2048, 2048, &m_d_write_text_layout);
    assert(SUCCEEDED(hr));

    DWRITE_TEXT_METRICS text_metrics = {};

    hr = m_d_write_text_layout->GetMetrics(&text_metrics);
    assert(SUCCEEDED(hr));

    hr = m_d_write_factory->CreateTextLayout(
        text.data(),         // Text to be laid out
        text.size(),         // Length of the text
        m_d_write_text_format, // Text format
        text_metrics.width,    // Use measured text width
        text_metrics.height,   // Use measured text height
        &m_d_write_text_layout // Output text layout object
    );
    assert(SUCCEEDED(hr));

    DWRITE_TEXT_RANGE tr = {};
    tr.length = 256;
    tr.startPosition = 0;

    hr = m_d_write_text_layout->SetFontSize(font_size, tr);
    assert(SUCCEEDED(hr));

    hr = m_d_write_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    assert(SUCCEEDED(hr));

    hr = m_d_write_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    assert(SUCCEEDED(hr));

    hr = m_FW1_factory->CreateFontWrapper(RendererDX11::get_instance_dx11()->get_device(), L"Arial", &m_font_wrapper);
    assert(SUCCEEDED(hr));

    m_FW1_factory->Release();
}

void ScreenText::draw() const
{
    m_font_wrapper->DrawTextLayout(
        RendererDX11::get_instance_dx11()->get_device_context(),
        m_d_write_text_layout,
        position.x,
        position.y,
        color,
        flags
    );
}

void ScreenText::draw_editor()
{
    ImGui::DragFloat2("Position", glm::value_ptr(position));
}

void ScreenText::set_text(std::wstring const& new_content)
{
    text = new_content;

    // Update the DirectWrite text layout if necessary
    if (m_d_write_text_layout != nullptr)
    {
        // Release the existing text layout
        m_d_write_text_layout->Release();
        m_d_write_text_layout = nullptr;
    }

    // Create a new text layout with the updated content
    HRESULT hr = m_d_write_factory->CreateTextLayout(new_content.data(), new_content.size(), m_d_write_text_format, 2048, 2048, &m_d_write_text_layout);
    assert(SUCCEEDED(hr));

    DWRITE_TEXT_METRICS text_metrics = {};

    hr = m_d_write_text_layout->GetMetrics(&text_metrics);
    assert(SUCCEEDED(hr));

    // Release mock layout needed for text size deduction.
    m_d_write_text_layout->Release();
    m_d_write_text_layout = nullptr;

    hr = m_d_write_factory->CreateTextLayout(
        text.data(),
        text.size(),
        m_d_write_text_format,
        text_metrics.width,
        text_metrics.height,
        &m_d_write_text_layout
    );
    assert(SUCCEEDED(hr));
}

D3D11_VIEWPORT ScreenText::get_viewport()
{
    UINT num_viewports = 1;
    D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    RendererDX11::get_instance_dx11()->get_device_context()->RSGetViewports(&num_viewports, viewports);
    return viewports[0];
}
