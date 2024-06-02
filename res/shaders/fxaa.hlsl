Texture2D screen_view : register(t0);
SamplerState quad_sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// These values are taken from:
// https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
#define EDGE_THRESHOLD_MIN 0.0312f
#define EDGE_THRESHOLD_MAX 0.125f

// It basically returns values from this list 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0.
// if q > 5
// e.g. QUALITY(6) = 1.5, QUALITY(7) = 2.0 etc.
#define QUALITY(q) ((q) < 5 ? 1.0f : ((q) > 5 ? ((q) < 10 ? 2.0f : ((q) < 11 ? 4.0f : 8.0f)) : 1.5f))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75f

float rgb_2_luma(float3 rgb)
{
    // Typical formula would be: sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));
    // However NVIDIA suggests calculating luma without the blue channel
    // as pure blue aliasing rarely appears in game content.
    // Our game is mostly blue-ish, yet using this formula
    // doesn't negatively impact anti-aliasing in our case either.
    return rgb.y * (0.587f / 0.299f) + rgb.x;
}

// This is pretty much this https://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
// rewritten in HLSL.
float3 FXAA(Texture2D tex, float2 UV)
{
    float3 color_in_center = tex.Sample(quad_sampler, UV).xyz;
    float luma_center = rgb_2_luma(color_in_center);

    float luma_down = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(0.0f, -1.0f)).rgb);
    float luma_up = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(0.0f, 1.0f)).rgb);
    float luma_left = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(-1.0f, 0.0f)).rgb);
    float luma_right = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(1.0f, 0.0f)).rgb);

    float luma_min = min(luma_center, min(min(luma_down, luma_up), min(luma_left, luma_right)));
    float luma_max = max(luma_center, max(max(luma_down, luma_up), max(luma_left, luma_right)));

    float luma_contrast = luma_max - luma_min;
    
    // If contrast is low, we're most likely not at the edge.
    if (luma_contrast < max(EDGE_THRESHOLD_MIN, luma_max * EDGE_THRESHOLD_MAX))
    {
        return color_in_center;
    }

    // Compute luma's for 3x3 field's corners.
    float luma_down_left = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(-1.0f, -1.0f)).rgb);
    float luma_up_right = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(1.0f, 1.0f)).rgb);
    float luma_up_left = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(-1.0f, 1.0f)).rgb);
    float luma_down_right = rgb_2_luma(tex.Sample(quad_sampler, UV, float2(1.0f, -1.0f)).rgb);

    float luma_down_up = luma_down + luma_up;
    float luma_left_right = luma_left + luma_right;

    float luma_left_corners = luma_down_left + luma_up_left;
    float luma_down_corners = luma_down_left + luma_down_right;
    float luma_right_corners = luma_down_right + luma_up_right;
    float luma_up_corners = luma_up_right + luma_up_left;
    
    // These two formulas are simplified from:
    // horizontal = |(upleft - left) - (left - downleft)| + 2 * |(up - center) - (center - down)| + |(upright - right) - (right - downright)|
    // verticla = |(upright - up) - (up - upleft)| + 2 * |(right - center) - (center - left)| + |(downright - down) - (down - downleft)|
    // And they are an estimation of gradients.
    float edge_horizontal = abs(-2.0f * luma_left + luma_left_corners) + abs(-2.0f * luma_center + luma_down_up ) * 2.0f + abs(-2.0f * luma_right + luma_right_corners);
    float edge_vertical = abs(-2.0f * luma_up + luma_up_corners) + abs(-2.0f * luma_center + luma_left_right) * 2.0f + abs(-2.0f * luma_down + luma_down_corners);

    // The edge is the one where the gradient is higher (so basically the luma is changing faster).
    bool is_horizontal = (edge_horizontal >= edge_vertical);

    float luma1 = is_horizontal ? luma_down : luma_left;
    float luma2 = is_horizontal ? luma_up : luma_right;
    float gradient1 = luma1 - luma_center;
    float gradient2 = luma2 - luma_center;

    // Find the steepest direction.
    bool is_1_steepest = abs(gradient1) >= abs(gradient2);

    // Normalized gradient in the steepest direction.
    float gradient_scaled = 0.25f * max(abs(gradient1), abs(gradient2));

    float2 screen_dimensions;
    screen_view.GetDimensions(screen_dimensions.x, screen_dimensions.y);
    float2 pixel_size = float2(1.0f / screen_dimensions.x, 1.0f / screen_dimensions.y);

    // Choose the step size (one pixel) according to the edge direction.
    float step_length = is_horizontal ? pixel_size.y : pixel_size.x;

    float luma_local_average = 0.0f;

    if (is_1_steepest)
    {
        // Switch the direction.
        step_length = -step_length;
        luma_local_average = 0.5f * (luma1 + luma_center);
    }
    else
    {
        luma_local_average = 0.5f * (luma2 + luma_center);
    }

    float2 current_UV = UV;
    // Shift UV in the correct direction by half a pixel.
    if (is_horizontal)
    {
        current_UV.y += step_length * 0.5f;
    }
    else
    {
        current_UV.x += step_length * 0.5f;
    }

    float2 offset = is_horizontal ? float2(pixel_size.x, 0.0f) : float2(0.0f, pixel_size.y);
    float2 uv_1 = current_UV - offset;
    float2 uv_2 = current_UV + offset;

    // Read the lumas at both current extremities of the exploration segment.
    float luma_end_1 = rgb_2_luma(tex.Sample(quad_sampler, uv_1).rgb);
    float luma_end_2 = rgb_2_luma(tex.Sample(quad_sampler, uv_2).rgb);
    
    luma_end_1 -= luma_local_average;
    luma_end_2 -= luma_local_average;

    // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
    bool reached1 = abs(luma_end_1) >= gradient_scaled;
    bool reached2 = abs(luma_end_2) >= gradient_scaled;
    bool reached_both = reached1 && reached2;

    if (!reached1)
    {
        uv_1 -= offset;
    }

    if (!reached2)
    {
        uv_2 += offset;
    }

    if (!reached_both)
    {
        for (int i = 2; i < ITERATIONS; i++)
        {
            if (!reached1)
            {
                luma_end_1 = rgb_2_luma(tex.Sample(quad_sampler, uv_1).rgb);
                luma_end_1 = luma_end_1 - luma_local_average;
            }
            if (!reached2)
            {
                luma_end_2 = rgb_2_luma(tex.Sample(quad_sampler, uv_2).rgb);
                luma_end_2 = luma_end_2 - luma_local_average;
            }

            reached1 = abs(luma_end_1) >= gradient_scaled;
            reached2 = abs(luma_end_2) >= gradient_scaled;
            reached_both = reached1 && reached2;

            // If the side is not reached, we continue to explore in this direction, with a variable quality.
            if (!reached1)
            {
                uv_1 -= offset * QUALITY(i);
            }
            if (!reached2)
            {
                uv_2 += offset * QUALITY(i);
            }

            if (reached_both)
            {
                break;
            }
        }
    }

    // Compute the distances to each extremity of the edge.
    float distance1 = is_horizontal ? (UV.x - uv_1.x) : (UV.y - uv_1.y);
    float distance2 = is_horizontal ? (uv_2.x - UV.x) : (uv_2.y - UV.y);

    bool is_direction_1 = distance1 < distance2;
    float distance_final = min(distance1, distance2);

    float edge_thickness = (distance1 + distance2);

    float pixel_offset = - distance_final / edge_thickness + 0.5f;

    bool is_luma_center_smaller = luma_center < luma_local_average;
    bool correct_variation = ((is_direction_1 ? luma_end_1 : luma_end_2) < 0.0f) != is_luma_center_smaller;
    float final_ofset = correct_variation ? pixel_offset : 0.0f;
    // Subpixel AA
    // Full weighted average of the luma over the 3x3 neighborhood.
    float luma_average = (1.0f / 12.0f) * (2.0f * (luma_down_up + luma_left_right) + luma_left_corners + luma_right_corners);
    float sub_pixel_offset1 = clamp(abs(luma_average - luma_center) / luma_contrast, 0.0f, 1.0f);
    float sub_pixel_offset2 = (-2.0f * sub_pixel_offset1 + 3.0f) * sub_pixel_offset1 * sub_pixel_offset1;
    float sub_pixel_offset_final = sub_pixel_offset2 * sub_pixel_offset2 * SUBPIXEL_QUALITY;

    final_ofset = max(final_ofset, sub_pixel_offset_final);
    float2 finalUV = UV;
    if (is_horizontal)
    {
        finalUV.y += final_ofset * step_length;
    }
    else
    {
        finalUV.x += final_ofset * step_length;
    }

    float3 final_color = tex.Sample(quad_sampler, finalUV).rgb;

    return final_color;
}

struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos: SV_Position;
    float2 UV : TEXCOORD0;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(FXAA(screen_view, input.UV).xyz, 1.0f);
}
