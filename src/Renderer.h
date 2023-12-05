#pragma once
#include <unordered_map>

#include "DirectionalLight.h"
#include "Drawable.h"
#include "Light.h"
#include "PointLight.h"
#include "SpotLight.h"

class Renderer
{
public:
    static std::shared_ptr<Renderer> create();
    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;

    void register_shader(std::shared_ptr<Shader> const& shader);
    void unregister_shader(std::shared_ptr<Shader> const& shader);

    void register_drawable(std::weak_ptr<Drawable> const& drawable);

    void register_light(std::shared_ptr<Light> const& light);
    void unregister_light(std::shared_ptr<Light> const& light);

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

    inline static std::shared_ptr<Renderer> instance;

    inline static std::vector<std::shared_ptr<PointLight>> point_lights = {};
    inline static std::vector<std::shared_ptr<SpotLight>> spot_lights = {};
    inline static std::shared_ptr<DirectionalLight> directional_light = {};

    std::vector<std::shared_ptr<Light>> lights = {};
    std::unordered_map<std::shared_ptr<Shader>, std::vector<std::weak_ptr<Drawable>>> shaders_map = {};

    // TODO: Retrieve this information from the shader
    // NOTE: This has to be the same value as the variable in a shader to work in all cases.
    int32_t max_point_lights = 4;
    int32_t max_spot_lights = 4;
};
