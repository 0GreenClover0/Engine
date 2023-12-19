#version 430 core

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct BoundingBox
{
    vec3 center;
    float padding1;
    vec3 extents;
    float padding2;
};

layout(std430, binding = 1) readonly buffer boundingBoxesBuffer
{
    BoundingBox boundingBoxes[];
};

layout(std430, binding = 2) writeonly buffer visibleInstancesBuffer
{
    uint visibleInstances[];
};

// First vec3 - normal, last float - offset
uniform vec4 frustumPlanes[6];

const vec3 CornerOffsets[8] = vec3[](
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0)
);

int PlaneNormalToIndex(vec3 normal)
{
    int index = 0;

    if (normal.z >= 0 )
        index |= 1;

    if (normal.y >= 0 )
        index |= 2;

    if (normal.x >= 0 )
        index |= 4;

    return index;
}

bool HalfPlaneTest(vec3 point, vec3 normal, float offset)
{
    const float distance = dot(point, normal) + offset;

    // distance < -0.02f => point is behind the plane
    return distance >= -0.02;
}

bool IsInOrForwardPlane(vec4 plane)
{
    const int index = PlaneNormalToIndex(plane.xyz);

    // Test the farthest point of the box from the plane
    // if it's behind the plane, then the entire box will be.
    return HalfPlaneTest(boundingBoxes[gl_GlobalInvocationID.x].center + boundingBoxes[gl_GlobalInvocationID.x].extents * CornerOffsets[index], plane.xyz, plane.w);
}

void main()
{
    uint i = gl_GlobalInvocationID.x;

    bool visible = IsInOrForwardPlane(frustumPlanes[0]) && IsInOrForwardPlane(frustumPlanes[1]) &&
        IsInOrForwardPlane(frustumPlanes[2]) && IsInOrForwardPlane(frustumPlanes[3]) &&
        IsInOrForwardPlane(frustumPlanes[4]) && IsInOrForwardPlane(frustumPlanes[5]);
    visibleInstances[i] = visible ? 1 : 0;
}
