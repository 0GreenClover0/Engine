#pragma once
#include "Drawable.h"
#include "FW1FontWrapper.h"
#include "RendererDX11.h"

#include "dwrite.h"
#pragma comment(lib, "Dwrite")

class ScreenText final : public Drawable
{
public:
    static std::shared_ptr<ScreenText> create();
    static std::shared_ptr<ScreenText> create(std::wstring const& content, glm::vec2 const& position, float const font_size,
                                              u32 const color, u16 const flags);

    ScreenText(AK::Badge<ScreenText>, std::shared_ptr<Material> const& material);
    ScreenText(AK::Badge<ScreenText>, std::wstring const& content, glm::vec2 const& position, float const font_size, u32 const color,
               u16 const flags);
    ~ScreenText() override;

    virtual void initialize() override;
    virtual void draw() const override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void reprepare() override;

    // Only content of the text component is now dynamically updated.
    void set_text(std::wstring const& new_content);

    // Text properties
    std::wstring text = L"Example text";
    glm::vec2 position = {};
    float font_size = 40;
    u32 color = 0;
    u16 flags = 0; //  Stores flags such as FW1_CENTER | FW1_VCENTER. FW1_RESTORESTATE is set by default.
    std::string font_name = {};
    bool bold = false;

private:
    static D3D11_VIEWPORT get_viewport();

    // DWrite and FW1 variables
    IFW1Factory* m_FW1_factory = nullptr;
    IFW1FontWrapper* m_font_wrapper = nullptr;
    IDWriteFactory* m_d_write_factory = nullptr;
    IDWriteTextFormat* m_d_write_text_format = nullptr;
    IDWriteTextLayout* m_d_write_text_layout = nullptr;

    // DirectX stuff
    D3D11_VIEWPORT m_viewport = {};
};
