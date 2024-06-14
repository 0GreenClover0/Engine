#pragma once

#include "Drawable.h"
#include "RendererDX11.h"

#include <FW1FontWrapper.h>

#include <dwrite.h>

#pragma comment(lib, "Dwrite")

class ScreenText final : public Drawable
{
public:
    static std::shared_ptr<ScreenText> create();
    static std::shared_ptr<ScreenText> create(std::shared_ptr<Material> const& material, std::string const& content,
                                              glm::vec2 const& position, float const font_size, u32 const color, u16 const flags);

    ScreenText(AK::Badge<ScreenText>, std::shared_ptr<Material> const& material, std::string const& content, glm::vec2 const& position,
               float const font_size, u32 const color, u16 const flags);
    ~ScreenText() override;

    virtual void initialize() override;
    virtual void draw() const override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    // This can be updated.
    void set_text(std::string const& new_content);
    void realign_text(bool const center) const;

    // Text properties
    std::string text = "Example text";
    glm::vec2 position = {};
    float font_size = 40;
    u32 color = 0;
    u16 flags = 0; //  Stores flags such as FW1_CENTER | FW1_VCENTER. FW1_RESTORESTATE is set by default.
    std::string font_name = {};
    bool bold = false;

private:
    static D3D11_VIEWPORT get_viewport();
    void refresh_layout();

    // DWrite and FW1 variables
    IFW1Factory* m_FW1_factory = nullptr;
    IFW1FontWrapper* m_font_wrapper = nullptr;
    IDWriteFactory* m_d_write_factory = nullptr;
    IDWriteTextFormat* m_d_write_text_format = nullptr;
    IDWriteTextLayout* m_d_write_text_layout = nullptr;
    float m_layout_width = 2048.0f;
    float m_layout_height = 2048.0f;
    bool m_align_to_center = true;

    // DirectX stuff
    D3D11_VIEWPORT m_viewport = {};
};
