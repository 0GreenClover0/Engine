#pragma once

#include <codecvt>
#include <iomanip>
#include <locale>
#include <memory>
#include <random>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include "Types.h"

namespace AK
{

#pragma region GUID_creation

// https://lowrey.me/guid-generation-in-c-11/
inline unsigned char random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

inline glm::vec4 interpolate_color(glm::vec4 const& start, glm::vec4 const& end, float const factor)
{
    float r = start.r + factor * (end.r - start.r);
    float g = start.g + factor * (end.g - start.g);
    float b = start.b + factor * (end.b - start.b);
    float a = start.a + factor * (end.a - start.a);
    return {r, g, b, a};
}

inline std::string generate_hex(u32 const length)
{
    std::stringstream ss;
    for (u32 i = 0; i < length; ++i)
    {
        auto const random_character = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << static_cast<i32>(random_character);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

inline std::wstring string_to_wstring(std::string const& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

inline glm::vec3 convert_2d_to_3d(glm::vec2 const& v, float desired_y = 0.0f)
{
    return {v.x, desired_y, v.y};
}

inline glm::vec2 convert_3d_to_2d(glm::vec3 const& v)
{
    return {v.x, v.z};
}

inline std::string generate_guid()
{
    std::string result;

    u32 constexpr guid_lengths[] = {8, 4, 4, 4, 12};

    for (u32 i = 0; i < 5; ++i)
    {
        result += generate_hex(guid_lengths[i]);
    }

    return result;
}

inline i32 random_int(i32 const min, i32 const max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

inline float random_float(float const min, float const max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

inline bool random_bool()
{
    return random_int(0, 1) == 1;
}

inline void extract_time(u32 const time, std::string& minutes, std::string& seconds)
{
    u32 const minutes_value = time / 60;
    u32 const seconds_value = time % 60;

    std::stringstream minutes_stream;
    minutes_stream << std::setw(2) << std::setfill('0') << minutes_value;
    minutes = minutes_stream.str();

    std::stringstream seconds_stream;
    seconds_stream << std::setw(2) << std::setfill('0') << seconds_value;
    seconds = seconds_stream.str();
}

inline glm::vec2 move_towards(glm::vec2 const current, glm::vec2 const target, float const max_distance_delta)
{
    if (max_distance_delta == 0.0f)
        return current;

    glm::vec2 const a = target - current;
    float const magnitude = glm::length(a);

    if (magnitude <= max_distance_delta || magnitude == 0.0f)
        return target;

    return current + a / magnitude * max_distance_delta;
}

inline glm::vec3 move_towards(glm::vec3 const current, glm::vec3 const target, float const max_distance_delta)
{
    if (max_distance_delta == 0.0f)
        return current;

    glm::vec3 const a = target - current;
    float const magnitude = glm::length(a);

    if (magnitude <= max_distance_delta || magnitude == 0.0f)
        return target;

    return current + a / magnitude * max_distance_delta;
}

inline u32 murmur_hash(u8 const* key, size_t const len, u32 const seed)
{
    u32 h = seed;
    if (len > 3)
    {
        u32 const* key_x4 = reinterpret_cast<u32 const*>(key);
        size_t i = len >> 2;
        do
        {
            u32 k = *key_x4++;
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
        } while (--i);
        key = reinterpret_cast<u8 const*>(key_x4);
    }
    if (len & 3)
    {
        size_t i = len & 3;
        u32 k = 0;
        key = &key[i - 1];
        do
        {
            k <<= 8;
            k |= *key--;
        } while (--i);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
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

template<typename T>
void swap_and_erase(std::vector<std::weak_ptr<T>>& vector, std::shared_ptr<T> element)
{
    for (u32 i = 0; i < vector.size(); ++i)
    {
        if (vector[i].expired())
            continue;

        if (vector[i].lock() == element)
        {
            std::swap(vector.at(i), vector.at(vector.size() - 1));
            vector.pop_back();
            return;
        }
    }
}

template<typename T>
void swap_and_erase(std::vector<T>& vector, size_t index)
{
    // NOTE: Swap with last and pop to avoid shifting other elements.
    std::swap(vector.at(index), vector.at(vector.size() - 1));
    vector.pop_back();
}

template<typename T>
void erase(std::vector<T>& vector, T element)
{
    if (auto const it = std::ranges::find(vector, element); it != vector.end())
    {
        vector.erase(it);
    }
}

template<typename T>
void erase(std::vector<std::weak_ptr<T>>& vector, std::shared_ptr<T> element)
{
    for (u32 i = 0; i < vector.size(); ++i)
    {
        if (vector[i].expired())
            continue;

        if (vector[i].lock() == element)
        {
            vector.erase(vector.begin() + i);
            return;
        }
    }
}

#pragma endregion

}
