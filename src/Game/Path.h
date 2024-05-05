#pragma once

#include "Component.h"
#include "Curve.h"

class Path final : public Curve
{
public:
    static std::shared_ptr<Path> create();

    explicit Path(AK::Badge<Path>);

    virtual void draw_editor() override;

private:
    float length() const;

    bool m_reverse_y = true;
};
