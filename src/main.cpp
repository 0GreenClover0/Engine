#include "Engine.h"

#define FORCE_DEDICATED_GPU 1

#if FORCE_DEDICATED_GPU
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int, char**)
{
    if (auto const result = Engine::initialize(); result != 0)
        return result;

    Engine::create_game();

    Engine::run();

    Engine::clean_up();

    return 0;
}
