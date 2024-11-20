#ifdef __cplusplus
#include <pxr/base/gf/Vec3f.h>

struct ISpectrum
{
    virtual pxr::GfVec3f toFloat3() = 0;
};

struct RGBSpectrum : ISpectrum
{
    pxr::GfVec3f data;

    pxr::GfVec3f toFloat3() override
    {
        return data;
    }
};
#else
interface ISpectrum
{
    // Convert the spectrum to a float3 representation
    float3 toFloat3();
}

struct RGBSpectrum : ISpectrum
{
    float3 data;

    float3 toFloat3()
    {
        return data;
    }
};
#endif