#pragma once
#include <string>

struct DialogueObject
{
    // If you add anything new here, don't forget to add serialization code in yaml-cpp-extensions.
    bool auto_end = true;
    std::string upper_line = "";
    std::string middle_line = "";
    std::string lower_line = "";

    std::string sound_path = "";
};
