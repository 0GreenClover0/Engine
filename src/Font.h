#pragma once

#include <string>
#include <vector>

struct Font
{
    std::string family_name = {};
    bool bold = false;

    std::vector<std::string> paths = {};
};
