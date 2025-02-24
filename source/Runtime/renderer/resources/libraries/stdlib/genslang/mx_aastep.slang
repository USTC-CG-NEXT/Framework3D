float mx_aastep(float threshold, float value)
{
    float afwidth = length(float2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
