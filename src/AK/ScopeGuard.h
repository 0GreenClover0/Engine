#pragma once

#include <utility>

namespace AK
{
template<typename Callback>
class ScopeGuard
{
public:
    ScopeGuard(Callback callback) : m_callback(std::move(callback))
    {
    }

    ~ScopeGuard()
    {
        m_callback();
    }

private:
    Callback m_callback;
};

template<typename Callback>
class ArmedScopeGuard
{
public:
    ArmedScopeGuard(Callback callback) : m_callback(std::move(callback))
    {
    }

    ~ArmedScopeGuard()
    {
        if (m_armed)
            m_callback();
    }

    void disarm()
    {
        m_armed = false;
    }

private:
    Callback m_callback;
    bool m_armed {true};
};

}

using AK::ArmedScopeGuard;
using AK::ScopeGuard;
