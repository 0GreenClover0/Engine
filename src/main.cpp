#include "Engine.h"

#define FORCE_DEDICATED_GPU 1

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#define WIN32_LEAN_AND_MEAN
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

#if FORCE_DEDICATED_GPU
extern "C"
{
    __declspec(dllexport) u32 NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) i32 AmdPowerXpressRequestHighPerformance = 1;
}
#endif

i32 main(i32, char**)
{
    if (auto const result = Engine::initialize(); result != 0)
        return result;

    Engine::create_game();

    Engine::run();

    Engine::clean_up();

    return 0;
}
