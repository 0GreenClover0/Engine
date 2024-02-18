#pragma once

namespace AK
{

template<typename T>
class Badge
{
public:
    using Type = T;

    // Absence of these doesn't work with make_shared
    //Badge(Badge const&) = delete;
    //Badge& operator=(Badge const&) = delete;

    //Badge(Badge&&) = delete;
    //Badge& operator=(Badge&&) = delete;

private:
    friend T;
    constexpr Badge() = default;
};

using AK::Badge;

}
