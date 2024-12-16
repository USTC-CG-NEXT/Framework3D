#ifdef __cplusplus
#include <pxr/base/gf/vec3f.h>

struct ISpectrum
{
    //virtual pxr::GfVec3f toFloat3() = 0;
};

struct RGBSpectrum : ISpectrum
{
    RGBSpectrum(const pxr::GfVec3f& data) : data(data) {}
    pxr::GfVec3f data;
    float padding;
};
#else
interface ISpectrum
{
    // Convert the spectrum to a float3 representation
    float3 toFloat3();
}

struct RGBSpectrum : ISpectrum
{
    __init(float v)
    {
        data = float3(v, v, v);
    }

    __init(float3 rgb)
    {
        data = rgb;
    }

    float3 data;
    float padding;

    [Differentiable]
    float3 toFloat3()
    {
        return data;
    }
};
#endif