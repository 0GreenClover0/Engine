#pragma once
#include <unordered_map>

#include "Drawable.h"

class Renderer
{
public:
    static std::shared_ptr<Renderer> create();
    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;

    void register_shader(std::shared_ptr<Shader> const& shader);
    void unregister_shader(std::shared_ptr<Shader> const& shader);

    void register_drawable(std::shared_ptr<Drawable> const& drawable);
    void unregister_drawable(std::shared_ptr<Drawable> const& drawable);

    void render() const;

    static std::shared_ptr<Renderer> get_instance()
    {
        return instance;
    }

private:
    Renderer() = default;

    static void set_instance(std::shared_ptr<Renderer> const& renderer)
    {
        instance = renderer;
    }

    std::unordered_map<std::shared_ptr<Shader>, std::vector<std::shared_ptr<Drawable>>> shaders_map;
    inline static std::shared_ptr<Renderer> instance;
};
