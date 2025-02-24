#pragma once

#include"PathTracer/Preprocessor.h"
#include "Geometry/ray.h"

#if!defined(__CUDACC__)
#include <algorithm>
#include <cmath>
using std::max;
using std::min;
using std::sqrt;
#endif

struct float3x3
{
    OPTIKA_HOSTDEVICE

    float3x3()
    {
        m_00 = 1;
        m_01 = 0;
        m_02 = 0;
        m_10 = 0;
        m_11 = 1;
        m_12 = 0;
        m_20 = 0;
        m_21 = 0;
        m_22 = 1;
    }

    //ʹ����������ʼ��
    OPTIKA_HOSTDEVICE

    float3x3(float3 v1, float3 v2, float3 v3)
    {
        m_00 = v1.x;
        m_10 = v1.y;
        m_20 = v1.z;
        m_01 = v2.x;
        m_11 = v2.y;
        m_21 = v2.z;
        m_02 = v3.x;
        m_12 = v3.y;
        m_22 = v3.z;
    }

    OPTIKA_HOSTDEVICE

    float3x3(const float3x3& in)
    {
        m_00 = in.m_00;
        m_01 = in.m_01;
        m_02 = in.m_02;
        m_10 = in.m_10;
        m_11 = in.m_11;
        m_12 = in.m_12;
        m_20 = in.m_20;
        m_21 = in.m_21;
        m_22 = in.m_22;
    }

    OPTIKA_HOSTDEVICE

    float3x3& operator=(const float3x3& in)
    {
        m_00 = in.m_00;
        m_01 = in.m_01;
        m_02 = in.m_02;
        m_10 = in.m_10;
        m_11 = in.m_11;
        m_12 = in.m_12;
        m_20 = in.m_20;
        m_21 = in.m_21;
        m_22 = in.m_22;

        return *this;
    }

    float m_00;
    float m_01;
    float m_02;
    float m_10;
    float m_11;
    float m_12;
    float m_20;
    float m_21;
    float m_22;
};

OPTIKA_INLINE OPTIKA_HOSTDEVICE
float3x3 Mul(const float3x3& A, const float3x3& B)
{
    float3x3 out;
    out.m_00 = A.m_00 * B.m_00 + A.m_01 * B.m_10 + A.m_02 * B.m_20;
    out.m_01 = A.m_00 * B.m_01 + A.m_01 * B.m_11 + A.m_02 * B.m_21;
    out.m_02 = A.m_00 * B.m_02 + A.m_01 * B.m_12 + A.m_02 * B.m_22;

    out.m_10 = A.m_10 * B.m_00 + A.m_11 * B.m_10 + A.m_12 * B.m_20;
    out.m_11 = A.m_10 * B.m_01 + A.m_11 * B.m_11 + A.m_12 * B.m_21;
    out.m_12 = A.m_10 * B.m_02 + A.m_11 * B.m_12 + A.m_12 * B.m_22;

    out.m_20 = A.m_20 * B.m_00 + A.m_21 * B.m_10 + A.m_22 * B.m_20;
    out.m_21 = A.m_20 * B.m_01 + A.m_21 * B.m_11 + A.m_22 * B.m_21;
    out.m_22 = A.m_20 * B.m_02 + A.m_21 * B.m_12 + A.m_22 * B.m_22;

    return out;
}

struct float4x4
{
    OPTIKA_HOSTDEVICE

    float4x4()
    {
        m_00 = 0;
        m_01 = 0;
        m_02 = 0;
        m_03 = 0;
        m_10 = 0;
        m_11 = 0;
        m_12 = 0;
        m_13 = 0;
        m_20 = 0;
        m_21 = 0;
        m_22 = 0;
        m_23 = 0;
        m_30 = 0;
        m_31 = 0;
        m_32 = 0;
        m_33 = 0;
    }

    float4x4(float s)
    {
        m_00 = s;
        m_01 = s;
        m_02 = s;
        m_03 = s;
        m_10 = s;
        m_11 = s;
        m_12 = s;
        m_13 = s;
        m_20 = s;
        m_21 = s;
        m_22 = s;
        m_23 = s;
        m_30 = s;
        m_31 = s;
        m_32 = s;
        m_33 = s;
    }

    OPTIKA_HOSTDEVICE

    float4x4(const float4x4& in)
    {
        m_00 = in.m_00;
        m_01 = in.m_01;
        m_02 = in.m_02;
        m_03 = in.m_03;
        m_10 = in.m_10;
        m_11 = in.m_11;
        m_12 = in.m_12;
        m_13 = in.m_13;
        m_20 = in.m_20;
        m_21 = in.m_21;
        m_22 = in.m_22;
        m_23 = in.m_23;
        m_30 = in.m_30;
        m_31 = in.m_31;
        m_32 = in.m_32;
        m_33 = in.m_33;
    }

    float4x4(const float4& v0, const float4& v1, const float4& v2, const float4& v3)
    {
        m_00 = v0.x;
        m_01 = v1.x;
        m_02 = v2.x;
        m_03 = v3.x;
        m_10 = v0.y;
        m_11 = v1.y;
        m_12 = v2.y;
        m_13 = v3.y;
        m_20 = v0.z;
        m_21 = v1.z;
        m_22 = v2.z;
        m_23 = v3.z;
        m_30 = v0.w;
        m_31 = v1.w;
        m_32 = v2.w;
        m_33 = v3.w;
    }


    OPTIKA_HOSTDEVICE

    float4x4(
        float m0,
        float m1,
        float m2,
        float m3,
        float m4,
        float m5,
        float m6,
        float m7,
        float m8,
        float m9,
        float m10,
        float m11,
        float m12,
        float m13,
        float m14,
        float m15)
    {
        m_00 = m0;
        m_01 = m1;
        m_02 = m2;
        m_03 = m3;
        m_10 = m4;
        m_11 = m5;
        m_12 = m6;
        m_13 = m7;
        m_20 = m8;
        m_21 = m9;
        m_22 = m10;
        m_23 = m11;
        m_30 = m12;
        m_31 = m13;
        m_32 = m14;
        m_33 = m15;
    }

    OPTIKA_HOSTDEVICE

    float4x4& operator=(const float4x4& in)
    {
        m_00 = in.m_00;
        m_01 = in.m_01;
        m_02 = in.m_02;
        m_03 = in.m_03;
        m_10 = in.m_10;
        m_11 = in.m_11;
        m_12 = in.m_12;
        m_13 = in.m_13;
        m_20 = in.m_20;
        m_21 = in.m_21;
        m_22 = in.m_22;
        m_23 = in.m_23;
        m_30 = in.m_30;
        m_31 = in.m_31;
        m_32 = in.m_32;
        m_33 = in.m_33;
        return *this;
    }

    OPTIKA_HOSTDEVICE

    bool operator==(const float4x4& in) const
    {
        if (m_00 != in.m_00)
            return false;
        if (m_01 != in.m_01)
            return false;
        if (m_02 != in.m_02)
            return false;
        if (m_03 != in.m_03)
            return false;
        if (m_10 != in.m_10)
            return false;
        if (m_11 != in.m_11)
            return false;
        if (m_12 != in.m_12)
            return false;
        if (m_13 != in.m_13)
            return false;
        if (m_20 != in.m_20)
            return false;
        if (m_21 != in.m_21)
            return false;
        if (m_22 != in.m_22)
            return false;
        if (m_23 != in.m_23)
            return false;
        if (m_30 != in.m_30)
            return false;
        if (m_31 != in.m_31)
            return false;
        if (m_32 != in.m_32)
            return false;
        if (m_33 != in.m_33)
            return false;

        return true;
    }

    OPTIKA_HOSTDEVICE

    float4x4 operator*(const float4x4& ma) const
    {
        float4x4 out;
        out.m_00 = m_00 * ma.m_00 + m_01 * ma.m_10 + m_02 * ma.m_20 + m_03 * ma.m_30;
        out.m_01 = m_00 * ma.m_01 + m_01 * ma.m_11 + m_02 * ma.m_21 + m_03 * ma.m_31;
        out.m_02 = m_00 * ma.m_02 + m_01 * ma.m_12 + m_02 * ma.m_22 + m_03 * ma.m_32;
        out.m_03 = m_00 * ma.m_03 + m_01 * ma.m_13 + m_02 * ma.m_23 + m_03 * ma.m_33;

        out.m_10 = m_10 * ma.m_00 + m_11 * ma.m_10 + m_12 * ma.m_20 + m_13 * ma.m_30;
        out.m_11 = m_10 * ma.m_01 + m_11 * ma.m_11 + m_12 * ma.m_21 + m_13 * ma.m_31;
        out.m_12 = m_10 * ma.m_02 + m_11 * ma.m_12 + m_12 * ma.m_22 + m_13 * ma.m_32;
        out.m_13 = m_10 * ma.m_03 + m_11 * ma.m_13 + m_12 * ma.m_23 + m_13 * ma.m_33;

        out.m_20 = m_20 * ma.m_00 + m_21 * ma.m_10 + m_22 * ma.m_20 + m_23 * ma.m_30;
        out.m_21 = m_20 * ma.m_01 + m_21 * ma.m_11 + m_22 * ma.m_21 + m_23 * ma.m_31;
        out.m_22 = m_20 * ma.m_02 + m_21 * ma.m_12 + m_22 * ma.m_22 + m_23 * ma.m_32;
        out.m_23 = m_20 * ma.m_03 + m_21 * ma.m_13 + m_22 * ma.m_23 + m_23 * ma.m_33;

        out.m_30 = m_30 * ma.m_00 + m_31 * ma.m_10 + m_32 * ma.m_20 + m_33 * ma.m_30;
        out.m_31 = m_30 * ma.m_01 + m_31 * ma.m_11 + m_32 * ma.m_21 + m_33 * ma.m_31;
        out.m_32 = m_30 * ma.m_02 + m_31 * ma.m_12 + m_32 * ma.m_22 + m_33 * ma.m_32;
        out.m_33 = m_30 * ma.m_03 + m_31 * ma.m_13 + m_32 * ma.m_23 + m_33 * ma.m_33;

        return out;
    }


    OPTIKA_HOSTDEVICE

    float4 operator*(const float4& vec) const
    {
        float4 out;
        out.x = m_00 * vec.x + m_01 * vec.y + m_02 * vec.z + m_03 * vec.w;
        out.y = m_10 * vec.x + m_11 * vec.y + m_12 * vec.z + m_13 * vec.w;
        out.z = m_20 * vec.x + m_21 * vec.y + m_22 * vec.z + m_23 * vec.w;
        out.w = m_30 * vec.x + m_31 * vec.y + m_32 * vec.z + m_33 * vec.w;
        return out;
    }

    float m_00;
    float m_01;
    float m_02;
    float m_03;
    float m_10;
    float m_11;
    float m_12;
    float m_13;
    float m_20;
    float m_21;
    float m_22;
    float m_23;
    float m_30;
    float m_31;
    float m_32;
    float m_33;
};


OPTIKA_INLINE OPTIKA_HOSTDEVICE void Inverse4x4Matrix(const float4x4& m, float4x4& im, float& det)
{
    float A2323 = m.m_22 * m.m_33 - m.m_23 * m.m_32;
    float A1323 = m.m_21 * m.m_33 - m.m_23 * m.m_31;
    float A1223 = m.m_21 * m.m_32 - m.m_22 * m.m_31;
    float A0323 = m.m_20 * m.m_33 - m.m_23 * m.m_30;
    float A0223 = m.m_20 * m.m_32 - m.m_22 * m.m_30;
    float A0123 = m.m_20 * m.m_31 - m.m_21 * m.m_30;
    float A2313 = m.m_12 * m.m_33 - m.m_13 * m.m_32;
    float A1313 = m.m_11 * m.m_33 - m.m_13 * m.m_31;
    float A1213 = m.m_11 * m.m_32 - m.m_12 * m.m_31;
    float A2312 = m.m_12 * m.m_23 - m.m_13 * m.m_22;
    float A1312 = m.m_11 * m.m_23 - m.m_13 * m.m_21;
    float A1212 = m.m_11 * m.m_22 - m.m_12 * m.m_21;
    float A0313 = m.m_10 * m.m_33 - m.m_13 * m.m_30;
    float A0213 = m.m_10 * m.m_32 - m.m_12 * m.m_30;
    float A0312 = m.m_10 * m.m_23 - m.m_13 * m.m_20;
    float A0212 = m.m_10 * m.m_22 - m.m_12 * m.m_20;
    float A0113 = m.m_10 * m.m_31 - m.m_11 * m.m_30;
    float A0112 = m.m_10 * m.m_21 - m.m_11 * m.m_20;
    det = m.m_00 * (m.m_11 * A2323 - m.m_12 * A1323 + m.m_13 * A1223) - m.m_01 * (
              m.m_10 * A2323 - m.m_12 * A0323 + m.m_13 * A0223) + m.m_02 * (
              m.m_10 * A1323 - m.m_11 * A0323 + m.m_13 * A0123) - m.m_03 * (
              m.m_10 * A1223 - m.m_11 * A0223 + m.m_12 * A0123);
    float inv_det = 1.0f / det;
    im.m_00 = inv_det * (m.m_11 * A2323 - m.m_12 * A1323 + m.m_13 * A1223);
    im.m_01 = inv_det * -(m.m_01 * A2323 - m.m_02 * A1323 + m.m_03 * A1223);
    im.m_02 = inv_det * (m.m_01 * A2313 - m.m_02 * A1313 + m.m_03 * A1213);
    im.m_03 = inv_det * -(m.m_01 * A2312 - m.m_02 * A1312 + m.m_03 * A1212);
    im.m_10 = inv_det * -(m.m_10 * A2323 - m.m_12 * A0323 + m.m_13 * A0223);
    im.m_11 = inv_det * (m.m_00 * A2323 - m.m_02 * A0323 + m.m_03 * A0223);
    im.m_12 = inv_det * -(m.m_00 * A2313 - m.m_02 * A0313 + m.m_03 * A0213);
    im.m_13 = inv_det * (m.m_00 * A2312 - m.m_02 * A0312 + m.m_03 * A0212);
    im.m_20 = inv_det * (m.m_10 * A1323 - m.m_11 * A0323 + m.m_13 * A0123);
    im.m_21 = inv_det * -(m.m_00 * A1323 - m.m_01 * A0323 + m.m_03 * A0123);
    im.m_22 = inv_det * (m.m_00 * A1313 - m.m_01 * A0313 + m.m_03 * A0113);
    im.m_23 = inv_det * -(m.m_00 * A1312 - m.m_01 * A0312 + m.m_03 * A0112);
    im.m_30 = inv_det * -(m.m_10 * A1223 - m.m_11 * A0223 + m.m_12 * A0123);
    im.m_31 = inv_det * (m.m_00 * A1223 - m.m_01 * A0223 + m.m_02 * A0123);
    im.m_32 = inv_det * -(m.m_00 * A1213 - m.m_01 * A0213 + m.m_02 * A0113);
    im.m_33 = inv_det * (m.m_00 * A1212 - m.m_01 * A0212 + m.m_02 * A0112);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Det(const float3x3& m)
{
    return m.m_00 * (m.m_11 * m.m_22 - m.m_21 * m.m_12) -
           m.m_01 * (m.m_10 * m.m_22 - m.m_12 * m.m_20) +
           m.m_02 * (m.m_10 * m.m_21 - m.m_11 * m.m_20);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE bool Inverse3x3Matrix(const float3x3& m, float3x3& Invm)
{
    double invdet = 1 / Det(m);

    if (invdet == 0)
        return false;

    Invm.m_00 = (m.m_11 * m.m_22 - m.m_21 * m.m_12) * invdet;
    Invm.m_01 = (m.m_02 * m.m_21 - m.m_01 * m.m_22) * invdet;
    Invm.m_02 = (m.m_01 * m.m_12 - m.m_02 * m.m_11) * invdet;
    Invm.m_10 = (m.m_12 * m.m_20 - m.m_10 * m.m_22) * invdet;
    Invm.m_11 = (m.m_00 * m.m_22 - m.m_02 * m.m_20) * invdet;
    Invm.m_12 = (m.m_10 * m.m_02 - m.m_00 * m.m_12) * invdet;
    Invm.m_20 = (m.m_10 * m.m_21 - m.m_20 * m.m_11) * invdet;
    Invm.m_21 = (m.m_20 * m.m_01 - m.m_00 * m.m_21) * invdet;
    Invm.m_22 = (m.m_00 * m.m_11 - m.m_10 * m.m_01) * invdet;

    return true;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE void Transpose3x3Matrix(const float3x3& m, float3x3& transposed)
{
    transposed.m_00 = m.m_00;
    transposed.m_01 = m.m_10;
    transposed.m_02 = m.m_20;
    transposed.m_10 = m.m_01;
    transposed.m_11 = m.m_11;
    transposed.m_12 = m.m_21;
    transposed.m_20 = m.m_02;
    transposed.m_21 = m.m_12;
    transposed.m_22 = m.m_22;
}

OPTIKA_HOSTDEVICE

inline void Getfloat3x3Part(const float* transform, float3x3& part)
{
    part.m_00 = transform[0];
    part.m_01 = transform[1];
    part.m_02 = transform[2];
    part.m_10 = transform[4];
    part.m_11 = transform[5];
    part.m_12 = transform[6];
    part.m_20 = transform[8];
    part.m_21 = transform[9];
    part.m_22 = transform[10];
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 Mul(const float3x3& m, const float3& v)
{
    float3 ret;
    ret.x = m.m_00 * v.x + m.m_01 * v.y + m.m_02 * v.z;
    ret.y = m.m_10 * v.x + m.m_11 * v.y + m.m_12 * v.z;
    ret.z = m.m_20 * v.x + m.m_21 * v.y + m.m_22 * v.z;
    return ret;
}


OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 RGBtoXYZ(const float3& rgb)
{
    float3 xyz;

    xyz.x = 0.412453f * rgb.x + 0.357580f * rgb.y + 0.180423f * rgb.z;
    xyz.y = 0.212671f * rgb.x + 0.715160f * rgb.y + 0.072169f * rgb.z;
    xyz.z = 0.019334f * rgb.x + 0.119193f * rgb.y + 0.950227f * rgb.z;

    return xyz;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float luminance(const float3& rgb)
{
    return RGBtoXYZ(rgb).y;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Clamp(float val, float low, float high)
{
    if (val < low)
        return low;
    else if (val > high)
        return high;
    else
        return val;
}


constexpr float Pi = 3.1415926535897932385;
constexpr float InvPi = 0.31830988618379067154;
constexpr float PiOver2 = 1.57079632679489661923;
constexpr float PiOver4 = 0.78539816339744830961;
//#define Pi      (3.1415926535897932385f )
//#define InvPi   (0.31830988618379067154f)
//#define PiOver2 (1.57079632679489661923f)
//#define PiOver4 (0.78539816339744830961f)
//#endif


OPTIKA_INLINE OPTIKA_HOSTDEVICE float degrees_to_radians(float degrees)
{
    return degrees * Pi / 180.0;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE bool SameHemisphere(const float3& w, const float3& wp)
{
    return w.z * wp.z > 0;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 UniformSampleHemisphere(float u, float v)
{
    float z = u;
    float r = sqrt(max((float)0, (float)1. - z * z));
    float phi = 2 * Pi * v;
    return make_float3(r * cos(phi), r * sin(phi), z);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 UniformSampleSphere(float u, float v)
{
    float z = 1 - 2 * u;
    float r = sqrt(max((float)0, (float)1. - z * z));
    float phi = 2 * Pi * v;
    return make_float3(r * cos(phi), r * sin(phi), z);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float2 UniformSampleDisk(float u, float v)
{
    float r = sqrt(u);
    float theta = 2 * Pi * v;
    return make_float2(r * cos(theta), r * sin(theta));
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float2 ConcentricSampleDisk(float u, float v)
{
    // Map uniform random numbers to $[-1,1]^2$
    float2 uOffset = make_float2(2 * u - 1, 2 * v - 1);

    // Handle degeneracy at the origin
    if (uOffset.x == 0 && uOffset.y == 0)
        return make_float2(0, 0);

    // Apply concentric mapping to point
    float theta, r;
    if (abs(uOffset.x) > abs(uOffset.y))
    {
        r = uOffset.x;
        theta = PiOver4 * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
    }
    return r * make_float2(cos(theta), sin(theta));
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 CosineSampleHemisphere(float u, float v)
{
    float2 d = ConcentricSampleDisk(u, v);
    float z = sqrt(max((float)0, 1 - d.x * d.x - d.y * d.y));
    return make_float3(d.x, d.y, z);
}


#if defined(__CUDACC__)
#include "optix_device.h"
OPTIKA_INLINE __device__ RayDesc GetWorldRay()
{
    return RayDesc(
        optixGetWorldRayOrigin(),
        optixGetWorldRayDirection(),
        optixGetRayTmin(),
        optixGetRayTmax());
}

OPTIKA_INLINE __device__ float3 GetCurrentPos()
{
    return optixGetWorldRayOrigin() + optixGetWorldRayDirection() * optixGetRayTmax();
}
#endif


OPTIKA_INLINE OPTIKA_HOSTDEVICE float CosTheta(const float3& w)
{
    return w.z;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Cos2Theta(const float3& w)
{
    return w.z * w.z;
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float AbsCosTheta(const float3& w)
{
    return abs(w.z);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Sin2Theta(const float3& w)
{
    return max((float)0, (float)1 - Cos2Theta(w));
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float SinTheta(const float3& w)
{
    return sqrt(Sin2Theta(w));
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float TanTheta(const float3& w)
{
    return SinTheta(w) / CosTheta(w);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Tan2Theta(const float3& w)
{
    return Sin2Theta(w) / Cos2Theta(w);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float3 Faceforward(const float3& v, const float3& v2)
{
    return (dot(v, v2) < 0.f) ? -v : v;
}


OPTIKA_INLINE OPTIKA_HOSTDEVICE float CosPhi(const float3& w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float SinPhi(const float3& w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Cos2Phi(const float3& w)
{
    return CosPhi(w) * CosPhi(w);
}

OPTIKA_INLINE OPTIKA_HOSTDEVICE float Sin2Phi(const float3& w)
{
    return SinPhi(w) * SinPhi(w);
}


__forceinline__ __device__ float3 toSRGB(const float3& c)
{
    float invGamma = 1.0f / 2.4f;
    float3 powed = make_float3(powf(c.x, invGamma), powf(c.y, invGamma), powf(c.z, invGamma));
    return make_float3(
        c.x < 0.0031308f ? 12.92f * c.x : 1.055f * powed.x - 0.055f,
        c.y < 0.0031308f ? 12.92f * c.y : 1.055f * powed.y - 0.055f,
        c.z < 0.0031308f ? 12.92f * c.z : 1.055f * powed.z - 0.055f);
}

//__forceinline__ __device__ float dequantizeUnsigned8Bits( const unsigned char i )
//{
//    enum { N = (1 << 8) - 1 };
//    return min((float)i / (float)N), 1.f)
//}
__forceinline__ __device__ unsigned char quantizeUnsigned8Bits(float x)
{
    x = clamp(x, 0.0f, 1.0f);
    enum
    {
        N = (1 << 8) - 1,
        Np1 = (1 << 8)
    };
    return (unsigned char)min((unsigned int)(x * (float)Np1), (unsigned int)N);
}

__forceinline__ __device__ uchar4 make_color(const float3& c)
{
    // first apply gamma, then convert to unsigned char
    float3 srgb = toSRGB(clamp(c, 0.0f, 1.0f));
    return make_uchar4(
        quantizeUnsigned8Bits(srgb.x),
        quantizeUnsigned8Bits(srgb.y),
        quantizeUnsigned8Bits(srgb.z),
        255u);
}

__forceinline__ __device__ uchar4 make_color_no_gamma(const float c)
{
    // first apply gamma, then convert to unsigned char
    return make_uchar4(
        quantizeUnsigned8Bits(c),
        quantizeUnsigned8Bits(c),
        quantizeUnsigned8Bits(c),
        255u);
}

__forceinline__ __device__ uchar4 make_color_no_gamma(const float3& c)
{
    // first apply gamma, then convert to unsigned char
    return make_uchar4(
        quantizeUnsigned8Bits(c.x),
        quantizeUnsigned8Bits(c.y),
        quantizeUnsigned8Bits(c.z),
        255u);
}


__forceinline__ __device__ uchar4 make_color_no_gamma(const float4& c)
{
    // first apply gamma, then convert to unsigned char
    return make_color_no_gamma(make_float3(c.x, c.y, c.z));
}


__forceinline__ __device__ uchar4 make_color(const float4& c)
{
    return make_color(make_float3(c.x, c.y, c.z));
}
