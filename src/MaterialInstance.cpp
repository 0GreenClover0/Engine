#include "MaterialInstance.h"

MaterialInstance::MaterialInstance(std::shared_ptr<Material> const& material) : material(material)
{
}

MaterialInstance::MaterialInstance(std::shared_ptr<Material> const& material, bool const is_gpu_instanced) : material(material), is_gpu_instanced(is_gpu_instanced)
{
}
