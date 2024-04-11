#pragma once

#include <string>
#include <vector>

enum class DebugType
{
    Log,
    Warning,
    Error
};

struct DebugMessage
{
    DebugType type = DebugType::Log;
    std::string text = {};
};

class Debug
{
public:
    static void log(std::string const& message, DebugType type = DebugType::Log);

    inline static std::vector<DebugMessage> debug_messages = {};
};
