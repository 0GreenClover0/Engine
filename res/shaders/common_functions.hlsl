float4 gamma_correction(float3 color, float strength = 1.5f)
{
    // If screen is too bright it might be because DirectX can do gamma correction by itself.
    // However, it works only in fullscreen so keep that in mind just in case.
    float gamma_factor = 1.0f / strength;
    float3 sRGB_color = float3(pow(color.x, gamma_factor), pow(color.y, gamma_factor), pow(color.z, gamma_factor));
    return float4(sRGB_color.xyz, 1.0f);
}

float3 reinhard_tonemapping(float3 color)
{
    return color / (color + float3(1.0f, 1.0f, 1.0f));
}

float3 exposure_tonemapping(float3 color, float exposure = 1.5f)
{
    return float3(1.0f, 1.0f, 1.0f) - exp(-color * exposure);
}

void solve_quadratic(float a, float b, float c, out float min_t, out float max_t)
{
    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        // No real solutions so return a degenerate result
        min_t = 0.0f;
        max_t = 0.0f;
        return;
    }

    // solving ax^2 + bx + c = 0
    float t = -0.5f * (b + sign(b) * sqrt(discriminant));
    float closestT = t / a;
    float furthestT = c / t;

    if (closestT > furthestT)
    {
        min_t = furthestT;
        max_t = closestT;
    }
    else
    {
        min_t = closestT;
        max_t = furthestT;
    }
}

float3 normal_blend(float3 n1, float3 n2)
{
    // Unpack
    n1 = n1 * 2.0f - 1.0f;
    n2 = n2 * 2.0f - 1.0f;

    float3x3 nBasis = float3x3(float3(n1.z, n1.y, -n1.x), // +90 degree rotation around y axis
                               float3(n1.x, n1.z, -n1.y), // -90 degree rotation around x axis
                               float3(n1.x, n1.y,  n1.z));

    return normalize(n2.x*nBasis[0] + n2.y*nBasis[1] + n2.z*nBasis[2]);
}
