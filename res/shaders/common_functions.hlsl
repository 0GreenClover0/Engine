float4 gamma_correction(float3 color)
{
    // If screen is too bright it might be because DirectX can do gamma correction by itself.
    // However, it works only in fullscreen so keep that in mind just in case.
    float gamma_factor = 1.0f / 2.2f;
    float3 sRGB_color = float3(pow(color.x, gamma_factor), pow(color.y, gamma_factor), pow(color.z, gamma_factor));
    return float4(sRGB_color.xyz, 1.0f);
}

float3 reinhard_tonemapping(float3 color)
{
    return color / (color + float3(1.0f, 1.0f, 1.0f));
}

float3 exposure_tonemapping(float3 color, float exposure)
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
