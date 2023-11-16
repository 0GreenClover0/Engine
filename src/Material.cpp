#include "Material.h"

Material::Material(std::shared_ptr<Shader> shader) : shader(std::move(shader))
{
	
}
