#pragma once

#include "AK/Badge.h"
#include "Drawable.h"
#include "Event.h"
#include "Mesh.h"

class Button : public Drawable
{
public:
    // To attach a function somewhere in the code (eg. in GameController) to an event of the button
    // (you want to make something happen on button press, for example) just make a weak_ptr reference to a button
    // in this component and on object on_enabled() inside this component ATTACH to the on_clicked event of the referenced button.
    // You need to unattach from the event on on_disabled().
    // To reference a button, you can simply drag the button component to an exposed field using our amazing editor.
    // Example: Check ScreenText.

    static std::shared_ptr<Button> create();
    explicit Button(AK::Badge<Button>, std::shared_ptr<Material> const& material);

    virtual void initialize() override;
    virtual void draw() const override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void reprepare() override;
    void prepare();

    Event<void()> on_hovered;
    Event<void()> on_unhovered;
    Event<void()> on_clicked;
    Event<void()> on_unclicked;

    // Detecting clicks and hovers happens only in game.
    virtual void update() override;
    void perform_hover_and_click_checks();

    bool is_hovered() const;
    bool is_pressed() const;

    std::string path_default = "./res/textures/white.jpg";
    std::string path_hovered = "./res/textures/light_gray.jpg";
    std::string path_pressed = "./res/textures/dark_gray.jpg";

    glm::vec2 top_left_corner = {};
    glm::vec2 top_right_corner = {};
    glm::vec2 bottom_left_corner = {};
    glm::vec2 bottom_right_corner = {};

private:
    void calculate_corners_position();
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;

    bool m_previous_frame_hovered = false;
    bool m_this_frame_hovered = false;
    bool m_previous_frame_clicked = false;
    bool m_this_frame_clicked = false;

    std::string m_path = path_default;

    std::shared_ptr<Mesh> m_mesh = {};
};
