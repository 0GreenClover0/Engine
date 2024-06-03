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

    bool interact() const;

    void set_type(FactoryType const type);

private:
    FactoryType type = FactoryType::Generator;

    std::weak_ptr<Model> model = {};
};
