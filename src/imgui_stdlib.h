// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/imgui/wiki/Useful-Extensions#cness

#pragma once

#include "EngineDefines.h"

#include <string>

#if EDITOR

#include <imgui.h>

namespace ImGui
{
// ImGui::InputText() with std::string
// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
IMGUI_API bool InputText(char const* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                         void* user_data = nullptr);
IMGUI_API bool InputTextMultiline(char const* label, std::string* str, ImVec2 const& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0,
                                  ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
IMGUI_API bool InputTextWithHint(char const* label, char const* hint, std::string* str, ImGuiInputTextFlags flags = 0,
                                 ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
}

#endif
