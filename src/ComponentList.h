#pragma once

#include "AK/Types.h"

#define ENUMERATE_COMPONENTS                                      \
    ENUMERATE_COMPONENT(Camera, "Camera")                         \
    ENUMERATE_COMPONENT(Cube, "Cube")                             \
    ENUMERATE_COMPONENT(DirectionalLight, "Directional Light")    \
    ENUMERATE_COMPONENT(Grass, "Grass")                           \
    ENUMERATE_COMPONENT(Model, "Model")                           \
    ENUMERATE_COMPONENT(PlayerInput, "Player Input")              \
    ENUMERATE_COMPONENT(PointLight, "Point Light")                \
    ENUMERATE_COMPONENT(ScreenText, "Screen Text")                \
    ENUMERATE_COMPONENT(Sphere, "Sphere")                         \
    ENUMERATE_COMPONENT(SpotLight, "Spot Light")                  \
    ENUMERATE_COMPONENT(Sprite, "Sprite")                         \
    ENUMERATE_COMPONENT(Sound, "Sound")                           \
    ENUMERATE_COMPONENT(SoundListener, "Sound Listener")
