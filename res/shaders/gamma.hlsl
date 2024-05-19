float4 gamma_correction(float3 color)
{
    // If screen is too bright it might be because DirectX can do gamma correction by itself.
    // However, it works only in fullscreen so keep that in mind just in case.
    float gamma_factor = 1.0f / 2.2f;
    float3 sRGB_color = float3(pow(color.x, gamma_factor), pow(color.y, gamma_factor), pow(color.z, gamma_factor));
    return float4(sRGB_color.xyz, 1.0f);
}
