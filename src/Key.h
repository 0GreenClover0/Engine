#pragma once

#include "AK/Types.h"

struct Key
{
    explicit Key(i32 const key);

    i32 key;

    [[nodiscard]] bool get_key() const;
    [[nodiscard]] bool get_key_down() const;

private:
    bool m_is_down_this_frame = false;
    bool m_was_down_last_frame = false;

    friend class Input;
};

inline Key::Key(i32 const key)
{
    this->key = key;
}

inline bool Key::get_key() const
{
    return m_is_down_this_frame;
}

inline bool Key::get_key_down() const
{
    return m_is_down_this_frame && !m_was_down_last_frame;
}
