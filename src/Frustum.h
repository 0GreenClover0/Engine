#pragma once

#include "Plane.h"

struct Frustum
{
    Plane top_plane;
    Plane bottom_plane;

    Plane right_plane;
    Plane left_plane;

    Plane far_plane;
    Plane near_plane;
};
