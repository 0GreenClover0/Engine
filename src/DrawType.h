#pragma once

enum class DrawType
{
    Points,
    Lines,
    LineStrip,
    LineLoop,
    Triangles,
    TriangleStrip,
    TriangleFan,
    Patches,
};

enum class DrawFunctionType
{
    Indexed,
    NotIndexed,
};
