#pragma once
#include <glm/fwd.hpp>
#include <string>

typedef glm::u32 hstring;

static const hstring initialFNV = 2166136261U;
static const hstring multipleFNV = 16777619U;

static hstring hash(std::string s)
{
	hstring result = initialFNV;
	const glm::u64 len = strlen(s.c_str());

	for (glm::u64 i = 0; i < len; i++)
	{
		result = (result ^ s[i]) * multipleFNV;
	}

	return result;
}