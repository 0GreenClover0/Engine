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

enum class RasterizerDrawType
{
    Default = 0, // Global state of PolygonMode
    Wireframe = 1,
    Solid = 2,
    None = 3,
};
