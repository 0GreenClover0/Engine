#include "RenderPassResourceContainer.h"

RenderPassResourceContainer::~RenderPassResourceContainer() = default;

void RenderPassResourceContainer::use_shader() const
{
    m_pass_shader->use();
}
