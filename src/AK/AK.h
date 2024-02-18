#pragma once
#include <memory>
#include <sstream>
#include <random>
#include <string>

namespace AK
{

template <typename T>
bool is_uninitialized(std::weak_ptr<T> const& weak)
{
    using wt = std::weak_ptr<T>;
    return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}

#pragma region GUID_creation

// https://lowrey.me/guid-generation-in-c-11/
inline unsigned char random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

inline std::string generate_hex(unsigned int const length)
{
    std::stringstream ss;
    for (uint32_t i = 0; i < length; ++i)
    {
        auto const random_character = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << static_cast<int>(random_character);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

inline std::string generate_guid()
{
    std::string result;

    uint32_t constexpr guid_lengths[] = { 8, 4, 4, 4, 12 };

    for (uint32_t i = 0; i < 5; ++i)
    {
        result += generate_hex(guid_lengths[i]);
    }

    return result;
}

template<typename T>
void swap_and_erase(std::vector<T>& vector, T element)
{
    if (auto const it = std::ranges::find(vector, element); it != vector.end())
    {
        // NOTE: Swap with last and pop to avoid shifting other elements.
        std::swap(vector.at(it - vector.begin()), vector.at(vector.size() - 1));
        vector.pop_back();
    }
}

#pragma endregion

}