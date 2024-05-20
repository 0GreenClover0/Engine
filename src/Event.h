#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

#include "AK/Types.h"

// Based on https://codereview.stackexchange.com/questions/188989/a-c-style-event-object-in-c

template<typename T>
// TODO: Handle move constructors
class Event
{
public:
    Event() = default;
    Event(Event const&) = delete;
    Event& operator=(Event const&) = delete;

    template<typename... Args>
    void operator()(Args&&... args)
    {
        std::lock_guard guard(mutex);

        if (m_listeners.empty())
            return;

        this->clean();

        for (auto listeners_copy = m_listeners; auto const& listener : listeners_copy)
        {
            if (auto locked = listener.first.lock())
            {
                listener.second(std::forward<Args>(args)...);
            }
        }
    }

    template<typename P, typename Q, typename R, typename... Args>
    void attach(P (Q::*f)(Args...), std::shared_ptr<R> const& p)
    {
        std::lock_guard guard(mutex);

        auto w = std::weak_ptr<Q>(std::static_pointer_cast<Q>(p));

        assert(!attached(w));

        auto l = [w, f](Args... args) {
            if (auto locked = w.lock())
            {
                return (*locked.get().*f)(args...);
            }

            return P();
        };

        m_listeners.emplace_back(std::weak_ptr<void>(w), l);
    }

    void detach(std::weak_ptr<void> const& p)
    {
        std::lock_guard guard(mutex);

        auto found = find(p);

        assert(found != m_listeners.end());

        if (found != m_listeners.end())
        {
            m_listeners.erase(found);
        }
    }

    [[nodiscard]] i32 count() const
    {
        std::lock_guard guard(mutex);

        return m_listeners.size();
    }

    std::mutex mutex;

protected:
    using event_pair = std::pair<std::weak_ptr<void>, std::function<T>>;

    bool attached(std::weak_ptr<void> const& p)
    {
        return find(p) != m_listeners.end();
    }

    void clean()
    {
        m_listeners.erase(
            std::remove_if(std::begin(m_listeners), std::end(m_listeners), [&](event_pair const& p) -> bool { return p.first.expired(); }),
            std::end(m_listeners));
    }

    typename std::vector<event_pair>::const_iterator find(std::weak_ptr<void> const& p) const
    {
        if (auto listener = p.lock())
        {
            return std::find_if(m_listeners.begin(), m_listeners.end(), [&listener](event_pair const& pair) {
                auto other = pair.first.lock();

                return other && other == listener;
            });
        }

        return m_listeners.end();
    }

    std::vector<event_pair> m_listeners;
};
