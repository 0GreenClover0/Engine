#pragma once

#include "Component.h"

#include "AK/Badge.h"
#include "PointLight.h"

class Model;

enum class FactoryType
{
    Generator,
    Workshop,
};

class Factory final : public Component
{
public:
    static std::shared_ptr<Factory> create();

    explicit Factory(AK::Badge<Factory>);
#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void awake() override;
    bool interact();

    void set_type(FactoryType const type);

    void turn_off_lights() const;

    void update_lights() const;

    void set_glowing(bool const is_glowing) const;

    i32 get_max_flash_count() const;

    FactoryType type = FactoryType::Generator;

    std::vector<std::weak_ptr<PointLight>> lights = {};
    std::weak_ptr<PointLight> factory_light = {};

private:
    std::weak_ptr<Model> model = {};

    inline static constexpr i32 m_max_flash_count = 3;
};
