#pragma once

#include <tuple>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <iostream>
#include <cassert>
#include <algorithm>

// Based on https://codereview.stackexchange.com/questions/188989/a-c-style-event-object-in-c

template<typename T>
// TODO: Handle move constructors
class Event
{
public:
    Event() = default;
    Event(Event const&) = delete;
    Event& operator= (Event const&) = delete;

    template<typename... Args>
    void operator()(Args&&... args)
    {
        std::lock_guard guard(mutex);

        if (this->listeners.empty())
            return;
        
        this->clean();

        for (auto listeners_copy = this->listeners; auto const& listener : listeners_copy)
        {
            if (auto locked = listener.first.lock())
            {
                listener.second(std::forward<Args>(args)...);
            }
        }
    }

    template<typename P, typename Q, typename R, typename... Args>
    void attach(P(Q::*f)(Args...), std::shared_ptr<R> const& p)
    {
        std::lock_guard guard(mutex);

        auto w = std::weak_ptr<Q>(std::static_pointer_cast<Q>(p));

        assert(!attached(w));

        auto l = [w, f](Args... args)
        {
            if (auto locked = w.lock())
            {
                return (*locked.get().*f)(args...);
            }

            return P();
        };

        listeners.emplace_back(std::weak_ptr<void>(w), l);
    }

    void detach(std::weak_ptr<void> const& p)
    {
        std::lock_guard guard(mutex);

        assert(attached(p));

        auto found = find(p);

        if (found != listeners.end())
        {
            listeners.erase(found);
        }
    }

    [[nodiscard]] int count() const
    {
        std::lock_guard guard(mutex);

        return this->listeners.size();
    }

    std::mutex mutex;

protected:
    using event_pair = std::pair<std::weak_ptr<void>, std::function<T>>;

    bool attached(std::weak_ptr<void> const& p)
    {
        return find(p) != listeners.end();
    }

    void clean()
    {
        listeners.erase(std::remove_if(std::begin(listeners), std::end(listeners), [&](event_pair const & p) -> bool {
            return p.first.expired();
        }), std::end(listeners));
    }

    typename std::vector<event_pair>::const_iterator find(std::weak_ptr<void> const& p) const
    {
        if (auto listener = p.lock())
        {
            return std::find_if(listeners.begin(), listeners.end(), [&listener](event_pair const& pair)
            {
                auto other = pair.first.lock();

                return other && other == listener;
            });
        }

        return listeners.end();
    }

    std::vector<event_pair> listeners;
};
