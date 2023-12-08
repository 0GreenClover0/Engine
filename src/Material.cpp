#include "Material.h"

Material::Material(std::shared_ptr<Shader> const& shader) : shader(shader)
{
}

Material::Material(std::shared_ptr<Shader> const& shader, bool const is_gpu_instanced) : shader(shader), is_gpu_instanced(is_gpu_instanced)
{
}
