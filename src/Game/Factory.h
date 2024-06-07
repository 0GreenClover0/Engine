#pragma once

#include "Component.h"

#include "AK/Badge.h"

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
    virtual void draw_editor() override;

    bool interact() const;

    void set_type(FactoryType const type);

    FactoryType type = FactoryType::Generator;

private:
    std::weak_ptr<Model> model = {};
};
