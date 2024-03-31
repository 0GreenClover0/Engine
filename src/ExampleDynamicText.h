#pragma once

#include "Component.h"
#include "Engine.h"
#include "ScreenText.h"

class ExampleDynamicText final : public Component
{
public:
    virtual void awake() override;
    virtual void update() override;

private:
    std::shared_ptr<ScreenText> m_foo = nullptr;
    std::wstring m_example = L"test";
};
