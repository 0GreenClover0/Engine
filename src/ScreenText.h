#pragma once
#include "Drawable.h"
#include "FW1FontWrapper.h"
#include "RendererDX11.h"

#include "dwrite.h"
#pragma comment(lib, "Dwrite")

class ScreenText final : public Drawable
{
public:
    explicit ScreenText(std::wstring const& content, glm::vec2 const& position, float const font_size, u32 const color, u16 const flags);
    ~ScreenText() override;

    virtual void start() override;
    virtual void draw() const override;
    virtual std::string get_name() const override;

    // Only content of the text component is now dynamically updated.
    void set_text(std::wstring const& new_content);

private:
    static D3D11_VIEWPORT get_viewport();

    // Text properties
    std::wstring m_text = {};
    glm::vec2 m_position = {};
    float m_font_size = 128;
    u32 m_color = 0;
    u16 m_flags = 0; //  Stores flags such as FW1_CENTER | FW1_VCENTER. FW1_RESTORESTATE is set by default.

    // DWrite and FW1 variables
    IFW1Factory* m_FW1_factory = nullptr;
    IFW1FontWrapper* m_font_wrapper = nullptr;
    IDWriteFactory* m_d_write_factory = nullptr;
    IDWriteTextFormat* m_d_write_text_format = nullptr;
    IDWriteTextLayout* m_d_write_text_layout = nullptr;

    // DirectX stuff
    D3D11_VIEWPORT m_viewport = {};
};
