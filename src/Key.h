#pragma once

#include <cstdint>

struct Key
{
    explicit Key(int32_t const key);

    int32_t key;

    [[nodiscard]] bool get_key() const;
    [[nodiscard]] bool get_key_down() const;

private:
    bool is_down_this_frame = false;
    bool was_down_last_frame = false;

    friend class Input;
};

inline Key::Key(int32_t const key)
{
    this->key = key;
}

inline bool Key::get_key() const
{
    return is_down_this_frame;
}

inline bool Key::get_key_down() const
{
    return is_down_this_frame && !was_down_last_frame;
}
