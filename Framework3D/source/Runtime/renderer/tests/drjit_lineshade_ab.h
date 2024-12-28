#pragma once

#include <corecrt_math_defines.h>

#include <chrono>
#include <complex>
#include <glm/glm.hpp>
#include <iostream>

#include "glints/glints_params.h"

#define TWOPOINTS

using Vector2f = glm::vec2;
using Vector2i = glm::ivec2;
using Vector3f = glm::vec3;
using Bool = bool;

std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

template<typename T>
inline T select(Bool cond, T true_, T false_)
{
    return cond ? true_ : false_;
}

struct PatchDr {
    Vector2f uv0, uv1, uv2, uv3;
    Vector2i pixel_center;

    // Storage this or calc this?
    Vector3f camera_pos_uv;
    Vector3f light_pos_uv;
};

struct LineDr {
    LineDr(const Vector2f& begin_point, const Vector2f& end_point)
        : begin_point(begin_point),
          end_point(end_point)
    {
    }

    Vector2f begin_point;
    Vector2f end_point;
};
using Float = float;
using Complex = std::complex<float>;
using LineDrFloat = LineDr;
using PatchDrFloat = PatchDr;

#define Sqrt(x)    sqrt(Complex(x))
#define GetReal(x) real(x)
#define Log        log

#define CalcPowerSeries(name)                     \
    Float name##_power2 = (name) * name;          \
    Float name##_power3 = (name) * name##_power2; \
    Float name##_power4 = (name) * name##_power3; \
    Float name##_power5 = (name) * name##_power4; \
    Float name##_power6 = (name) * name##_power5; \
    Float name##_power7 = (name) * name##_power6; \
    Float name##_power8 = (name) * name##_power7;

#define CalcPowerSeriesComplex(name)                \
    Complex name##_power2 = (name) * name;          \
    Complex name##_power3 = (name) * name##_power2; \
    Complex name##_power4 = (name) * name##_power3; \
    Complex name##_power5 = (name) * name##_power4; \
    Complex name##_power6 = (name) * name##_power5; \
    Complex name##_power7 = (name) * name##_power6; \
    Complex name##_power8 = (name) * name##_power7;

#define DeclarePowerSeries(name)                                               \
    Float name, Float name##_power2, Float name##_power3, Float name##_power4, \
        Float name##_power5, Float name##_power6, Float name##_power7,         \
        Float name##_power8

#define UsePowerSeries(name)                                          \
    name, name##_power2, name##_power3, name##_power4, name##_power5, \
        name##_power6, name##_power7, name##_power8

#undef Power

#define Power(name, n) name##_power##n

#define work_for_div 100000000.f

inline Complex sumpart_coeff_a(
    Complex y,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeriesComplex(y);

    auto m =
        (halfX * halfZ * (-1 + Power(halfZ, 2) * Power(r, 2)) *
             (-10 + (11 * Power(halfX, 2) + 9 * Power(halfZ, 2)) * Power(r, 2) +
              Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                  Power(r, 4)) *
             Power(width, 3) -
         4 *
             (Power(halfZ, 2) * powf(-1 + Power(halfZ, 2) * Power(r, 2), 3) +
              2 * Power(halfX, 4) * Power(halfZ, 2) * Power(r, 4) *
                  (11 + Power(halfZ, 2) * Power(r, 2)) +
              Power(halfX, 2) * (1 - 21 * Power(halfZ, 2) * Power(r, 2) +
                                 17 * Power(halfZ, 4) * Power(r, 4) +
                                 3 * Power(halfZ, 6) * Power(r, 6))) *
             Power(width, 2) * y +
         4 * halfX * halfZ *
             (22 + 3 * (-23 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
              2 *
                  (22 * Power(halfX, 4) +
                   7 * Power(halfX, 2) * Power(halfZ, 2) -
                   14 * Power(halfZ, 4)) *
                  Power(r, 4) +
              Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                  (4 * Power(halfX, 2) + 3 * Power(halfZ, 2)) * Power(r, 6)) *
             width * Power(y, 2) +
         64 * (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
             (Power(halfX, 2) - Power(halfZ, 2) +
              Power(halfZ, 2) * (5 * Power(halfX, 2) + Power(halfZ, 2)) *
                  Power(r, 2)) *
             Power(y, 3)) *
        work_for_div;

    auto n = (4 * Power(halfX, 2) * Power(r, 2) * Power(width, 2) * y +
              8 * (-1 + Power(halfZ, 2) * Power(r, 2)) * Power(y, 3) -
              2 * Power(width, 2) * (y + Power(halfZ, 2) * Power(r, 2) * y) -
              halfX * halfZ * Power(r, 2) * width *
                  (Power(width, 2) - 12.f * Power(y, 2))) *
             work_for_div;

    // if (cuda::std::length(n) < 1e-10 )
    //{
    //     n = n / cuda::std::length(n) * 1e-10f;
    // }

    return m / n;
}

inline Complex sumpart_coeff_b(
    Complex y,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeriesComplex(y);

    auto m = (((powf(halfZ - Power(halfZ, 3) * Power(r, 2), 2) +
                Power(halfX, 2) * (-1 + Power(halfZ, 4) * Power(r, 4))) *
                   Power(width, 3) -
               4 * halfX * halfZ *
                   (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
                    Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                        Power(r, 4)) *
                   Power(width, 2) * y +
               4 *
                   (-7 * Power(halfX, 2) + 7 * Power(halfZ, 2) +
                    2 *
                        (3 * Power(halfX, 4) -
                         2 * Power(halfX, 2) * Power(halfZ, 2) -
                         4 * Power(halfZ, 4)) *
                        Power(r, 2) +
                    Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                        (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
                   width * Power(y, 2) +
               64 * halfX * halfZ *
                   (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
                   Power(y, 3))

                  ) *
             work_for_div;

    auto n = (halfX * halfZ * Power(r, 2) * Power(width, 3) +
              2 * (1 + (-2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
                  Power(width, 2) * y -
              12 * halfX * halfZ * Power(r, 2) * width * Power(y, 2) -
              8 * (-1 + Power(halfZ, 2) * Power(r, 2)) * Power(y, 3)) *
             work_for_div;

    return -m / n;
}

inline auto calc_res_a(
    Float x,
    Float a,
    Float b,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeries(x);

    auto rest =
        16 * (-1 + Power(halfZ, 2) * Power(r, 2)) *
            (b * (-1 + Power(halfZ, 2) * Power(r, 2)) -
             4 * a * halfX * halfZ * Power(r, 2) * width) *
            x +
        8 * a * powf(-1 + Power(halfZ, 2) * Power(r, 2), 2) * Power(x, 2);

    auto m =
        (4 * Power(r, 2) * Power(width, 2) *
         (-2 * b * (-1 + Power(halfZ, 2) * Power(r, 2)) *
              (halfX * halfZ * (-1 + Power(halfZ, 2) * Power(r, 2)) *
                   (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
                    Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                        Power(r, 4)) *
                   Power(width, 3) -
               2 *
                   (Power(halfZ, 2) *
                        powf(-1 + Power(halfZ, 2) * Power(r, 2), 3) +
                    4 * Power(halfX, 4) * Power(halfZ, 2) * Power(r, 4) *
                        (3 + Power(halfZ, 2) * Power(r, 2)) +
                    Power(halfX, 2) * (1 - 9 * Power(halfZ, 2) * Power(r, 2) +
                                       3 * Power(halfZ, 4) * Power(r, 4) +
                                       5 * Power(halfZ, 6) * Power(r, 6))) *
                   Power(width, 2) * x +
               4 * halfX * halfZ *
                   (2 +
                    3 * (-5 * Power(halfX, 2) + 3 * Power(halfZ, 2)) *
                        Power(r, 2) +
                    6 *
                        (2 * Power(halfX, 4) -
                         Power(halfX, 2) * Power(halfZ, 2) -
                         2 * Power(halfZ, 4)) *
                        Power(r, 4) +
                    Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                        (4 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 6)) *
                   width * Power(x, 2) +
               8 *
                   ((1 + Power(halfZ, 2) * Power(r, 2)) *
                        powf(halfZ - Power(halfZ, 3) * Power(r, 2), 2) +
                    2 * Power(halfX, 4) *
                        (Power(r, 2) + 6 * Power(halfZ, 2) * Power(r, 4) +
                         Power(halfZ, 4) * Power(r, 6)) +
                    Power(halfX, 2) * (-1 - 11 * Power(halfZ, 2) * Power(r, 2) +
                                       9 * Power(halfZ, 4) * Power(r, 4) +
                                       3 * Power(halfZ, 6) * Power(r, 6))) *
                   Power(x, 3)) +
          a * width *
              (32 * Power(halfX, 6) * Power(r, 4) *
                   (1 + 6 * Power(halfZ, 2) * Power(r, 2) +
                    Power(halfZ, 4) * Power(r, 4)) *
                   width * Power(x, 2) +
               Power(halfZ, 2) * powf(-1 + Power(halfZ, 2) * Power(r, 2), 2) *
                   width *
                   ((-1 + Power(halfZ, 4) * Power(r, 4)) * Power(width, 2) -
                    4 *
                        (1 + 6 * Power(halfZ, 2) * Power(r, 2) +
                         Power(halfZ, 4) * Power(r, 4)) *
                        Power(x, 2)) +
               16 * Power(halfX, 5) * halfZ * Power(r, 4) * x *
                   (-((1 + 6 * Power(halfZ, 2) * Power(r, 2) +
                       Power(halfZ, 4) * Power(r, 4)) *
                      Power(width, 2)) +
                    2 *
                        (5 + 10 * Power(halfZ, 2) * Power(r, 2) +
                         Power(halfZ, 4) * Power(r, 4)) *
                        Power(x, 2)) +
               2 * halfX * halfZ * powf(-1 + Power(halfZ, 2) * Power(r, 2), 2) *
                   x *
                   ((2 - 5 * Power(halfZ, 2) * Power(r, 2) -
                     5 * Power(halfZ, 4) * Power(r, 4)) *
                        Power(width, 2) +
                    4 *
                        (2 + 15 * Power(halfZ, 2) * Power(r, 2) +
                         3 * Power(halfZ, 4) * Power(r, 4)) *
                        Power(x, 2)) -
               2 * Power(halfX, 3) * halfZ * Power(r, 2) *
                   (-1 + Power(halfZ, 2) * Power(r, 2)) * x *
                   ((1 + 50 * Power(halfZ, 2) * Power(r, 2) +
                     13 * Power(halfZ, 4) * Power(r, 4)) *
                        Power(width, 2) -
                    4 *
                        (19 + 54 * Power(halfZ, 2) * Power(r, 2) +
                         7 * Power(halfZ, 4) * Power(r, 4)) *
                        Power(x, 2)) +
               2 * Power(halfX, 4) * Power(r, 2) * width *
                   ((-1 - 5 * Power(halfZ, 2) * Power(r, 2) +
                     5 * Power(halfZ, 4) * Power(r, 4) +
                     Power(halfZ, 6) * Power(r, 6)) *
                        Power(width, 2) +
                    8 *
                        (-2 - 21 * Power(halfZ, 2) * Power(r, 2) +
                         4 * Power(halfZ, 4) * Power(r, 4) +
                         3 * Power(halfZ, 6) * Power(r, 6)) *
                        Power(x, 2)) +
               Power(halfX, 2) * width *
                   (powf(-1 + Power(halfZ, 2) * Power(r, 2), 2) *
                        (1 + 12 * Power(halfZ, 2) * Power(r, 2) +
                         3 * Power(halfZ, 4) * Power(r, 4)) *
                        Power(width, 2) +
                    4 *
                        (1 + 38 * Power(halfZ, 2) * Power(r, 2) -
                         12 * Power(halfZ, 4) * Power(r, 4) -
                         30 * Power(halfZ, 6) * Power(r, 6) +
                         3 * Power(halfZ, 8) * Power(r, 8)) *
                        Power(x, 2))))) *
        work_for_div;
    auto n = ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              ((1 + halfZ * r) * Power(width, 2) - 4 * halfX * r * width * x -
               4 * (-1 + halfZ * r) * Power(x, 2)) *
              ((-1 + halfZ * r) * Power(width, 2) - 4 * halfX * r * width * x -
               4 * (1 + halfZ * r) * Power(x, 2))) *
             work_for_div;
    std::cout << "m: " << m << " n: " << n << std::endl;

    // if (abs(n) < 1E-16)
    //{
    //     n = n / abs(n) * 1E-16;
    // }

    std::cout << "rest: " << rest << std::endl;

    return rest + m / n;
}

Float cross_2d(const Vector2f& a, const Vector2f& b)
{
    return a.x * b.y - a.y * b.x;
}

inline Float lineShadeAB(
    Float lower[],
    Float upper[],
    Float a[],
    Float b[],
    Float alpha,
    Float halfX,
    Float halfZ,
    Float width)
{
    Float r = sqrt(1 - alpha * alpha);

    CalcPowerSeries(width);
    CalcPowerSeries(halfX);
    CalcPowerSeries(halfZ);
    CalcPowerSeries(r);

    Complex temp = Sqrt(
        -Power(width, 2) + Power(halfX, 2) * Power(r, 2) * Power(width, 2) +
        Power(halfZ, 2) * Power(r, 2) * Power(width, 2));

    Complex c[] = {
        (-(halfX * r * width) - temp) / (Float(2.) * (-1 + halfZ * r)),
        (-(halfX * r * width) - temp) / (Float(2.) * (1 + halfZ * r)),
        (-(halfX * r * width) + temp) / (Float(2.) * (-1 + halfZ * r)),
        (-(halfX * r * width) + temp) / (Float(2.) * (1 + halfZ * r))
    };

    auto ret_a = Complex(0, 0);
    auto ret_b = Complex(0, 0);

    for (int i = 0; i < 4; i++) {
        auto coeff_b = sumpart_coeff_b(
            c[i],
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r));

        auto coeff_a = sumpart_coeff_a(
            c[i],
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r));
        for (int j = 0; j < 4; ++j) {
            auto log_val_u = log(upper[j] - c[i]);
            auto log_val_l = log(lower[j] - c[i]);
            auto part_b = b[j] * (log_val_u - log_val_l) * coeff_b;
            ret_b += part_b;

            auto part_a = a[j] * (log_val_u - log_val_l) * coeff_a;
            ret_a += part_a;
        }
    }

    // printf("a is %f,b is %f\n", GetReal(ret_a), GetReal(ret_b));

    auto temp_1 = (Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) * width) /
                  (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2));

    ret_b *= temp_1;
    ret_a *= temp_1 * width;

    Float res = 0;

    for (int j = 0; j < 4; ++j) {
        auto temp =
            (calc_res_a(
                 upper[j],
                 a[j],
                 b[j],
                 UsePowerSeries(width),
                 UsePowerSeries(halfX),
                 UsePowerSeries(halfZ),
                 UsePowerSeries(r)) -
             calc_res_a(
                 lower[j],
                 a[j],
                 b[j],

                 UsePowerSeries(width),
                 UsePowerSeries(halfX),
                 UsePowerSeries(halfZ),
                 UsePowerSeries(r)));

        res += temp;
    }

    ret_a += res;

    Float coeff_b =
        -alpha * alpha /
        (Float(8.) * M_PI * powf(-1 + Power(halfZ, 2) * Power(r, 2), 3));
    Float coeff_a = powf(alpha, 2) /
                    (16. * M_PI * powf(-1 + Power(halfZ, 2) * Power(r, 2), 4));
    ret_a *= coeff_a;
    ret_b *= coeff_b;

    std::cout << ret_a.real() << " " << ret_a.imag() << std::endl;
    std::cout << ret_b.real() << " " << ret_b.imag() << std::endl;

    return GetReal(ret_b) + GetReal(ret_a);
}

inline Vector2f signed_areaAB(LineDrFloat line, Vector2f point)
{
    // The direction is expected to be normalized

    auto line_pos = (line.begin_point + line.end_point) / 2.f;
    auto line_direction = normalize(line.end_point - line.begin_point);
    auto distance = point - line_pos;

    auto x = cross_2d(distance, line_direction);
    auto y = dot(distance, line_direction);

    return Vector2f(x, y);
}

inline Float slope(Vector2f p1, Vector2f p2)
{
    return (p1.y - p2.y) / (p1.x - p2.x);
}

inline Float intercept(Vector2f p1, Vector2f p2)
{
    return (p1.x * p2.y - p2.x * p1.y) / (p1.x - p2.x);
}

inline Float areaIntegrate(Float x, Float a, Float b)
{
    return 0.5 * a * x * x + b * x;
}

inline Float areaCalc(Float lower[], Float upper[], Float a[], Float b[])
{
    Float ret = 0;
    for (int i = 0; i < 4; ++i) {
        ret += areaIntegrate(upper[i], a[i], b[i]) -
               areaIntegrate(lower[i], a[i], b[i]);
    }

    return abs(ret);
}

inline Float AbsCosTheta(Vector3f w)
{
    return abs(w.z);
}

inline Float Lum(Vector3f color)
{
    Vector3f YWeight(0.212671f, 0.715160f, 0.072169f);
    return dot(color, YWeight);
}

inline Float SchlickR0FromEta(Float eta)
{
    return (eta - 1) * (eta - 1) / ((eta + 1) * (eta + 1));
}

inline Float Cos2Theta(Vector3f w)
{
    return w.z * w.z;
}

inline Float Sin2Theta(Vector3f w)
{
    return 1 - Cos2Theta(w);
}

inline Float Tan2Theta(Vector3f w)
{
    return Sin2Theta(w) / Cos2Theta(w);
}

inline Float SinTheta(Vector3f w)
{
    return sqrt(Sin2Theta(w));
}

inline Float CosTheta(Vector3f w)
{
    return w.z;
}

inline Float TanTheta(Vector3f w)
{
    return SinTheta(w) / CosTheta(w);
}
using std::clamp;
inline Float CosPhi(Vector3f w)
{
    auto sinTheta = SinTheta(w);
    auto tmp = clamp(w.x / sinTheta, -1.f, 1.f);
    auto result = select(sinTheta == 0.f, 0.f, tmp);
    return result;
}

inline Float SinPhi(Vector3f w)
{
    auto sinTheta = SinTheta(w);
    auto tmp = clamp(w.y / sinTheta, -1.f, 1.f);
    auto result = select(sinTheta == 0.f, 0.f, tmp);
    return result;
}

inline Float Cos2Phi(Vector3f w)
{
    return CosPhi(w) * CosPhi(w);
}

inline Float Sin2Phi(Vector3f w)
{
    return SinPhi(w) * SinPhi(w);
}

inline Float SchlickWeight(Float cosTheta)
{
    Float m = clamp(1.f - cosTheta, 0.f, 1.f);
    return (m * m) * (m * m) * m;
}

template<typename T>
inline T lerp(T v0, T v1, Float t)
{
    return (1 - t) * v0 + t * v1;
}

inline Vector3f FrSchlick(Vector3f R0, Float cosTheta)
{
    return lerp(R0, Vector3f(1.f, 1.f, 1.f), SchlickWeight(cosTheta));
}

inline Vector3f
DisneyFresnel(Vector3f R0, Float metallic, Float eta, Float cosI)
{
    return FrSchlick(R0, cosI);
}

inline Vector3f Faceforward(Vector3f v1, Vector3f v2)
{
    auto tmp = dot(v1, v2);
    auto result = select(tmp < 0.f, -v1, v1);
    return result;
}

inline Float Microfacet_G1(Vector3f w, Vector2f param)
{
    Float absTanTheta = abs(TanTheta(w));
    auto alpha =
        sqrt(Cos2Phi(w) * param.x * param.x + Sin2Phi(w) * param.y * param.y);
    Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    Float lambda = (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2.f;
    return 1.f / (1.f + lambda);
}

inline Float Microfacet_G(Vector3f wi, Vector3f wo, Vector2f param)
{
    return Microfacet_G1(wi, param) * Microfacet_G1(wo, param);
}

inline Vector2f MakeMicroPara(Float roughness)
{
    Float ax = std::max(0.001f, sqrt(roughness));
    Float ay = std::max(0.001f, sqrt(roughness));
    Vector2f micro_para(ax, ay);

    return micro_para;
}

inline Float MicrofacetDistribution(Vector3f wh, Vector2f param)
{
    Float tan2Theta = Tan2Theta(wh);
    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    Float e = (Cos2Phi(wh) / (param.x * param.x) +
               Sin2Phi(wh) / (param.y * param.y)) *
              tan2Theta;
    return 1 / (M_PI * param.x * param.y * cos4Theta * (1 + e) * (1 + e));
}

inline Float bsdf_f(
    Vector3f ray_in_d,
    Vector3f ray_out_d,
    Float roughness,
    Vector3f baseColor = Vector3f(1.f))
{
    Vector2f micro_para = MakeMicroPara(roughness);

    Vector3f wo = normalize(ray_in_d), wi = normalize(ray_out_d);

    wo = select(wo.z < 0.f, wo * -1.f, wo);
    wi = select(wi.z < 0.f, wi * -1.f, wi);

    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0.f || cosThetaO == 0.f)
        return 0.f;
    // return make_float3(0.);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
        return 0.f;
    // return make_float3(0.);

    wh = normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    Float lum = Lum(baseColor);

    // normalize lum. to isolate hue+sat
    auto Ctint = select(
        lum > 0.f,
        Vector3f(baseColor.x / lum, baseColor.y / lum, baseColor.z / lum),
        Vector3f(1.f, 1.f, 1.f));

    auto Cspec0 = baseColor;
    // Lerp(metalness, SchlickR0FromEta(eta) * Lerp(specTint, make_float3(1.),
    // Ctint), baseColor);

    auto F = DisneyFresnel(
        Cspec0, 1.f, 0.f, dot(wi, Faceforward(wh, Vector3f(0.f, 0.f, 1.f))));

    return lum * MicrofacetDistribution(wh, micro_para) *
           Microfacet_G(wo, wi, micro_para) * Lum(F) /
           (4.f * cosThetaI * cosThetaO);
}

inline Float bsdf_f_line(
    Vector3f ray_in_d,
    Vector3f ray_out_d,
    Float roughness,
    Vector3f baseColor = Vector3f(1.f))
{
    Vector2f micro_para = MakeMicroPara(roughness);

    Vector3f wo = normalize(ray_in_d), wi = normalize(ray_out_d);

    wo = select(wo.z < 0.f, wo * -1.f, wo);
    wi = select(wi.z < 0.f, wi * -1.f, wi);

    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0.f || cosThetaO == 0.f)
        return 0.f;
    // return make_float3(0.);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
        return 0.f;
    // return make_float3(0.);

    wh = normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    Float lum = Lum(baseColor);

    // normalize lum. to isolate hue+sat
    auto Ctint = select(
        lum > 0.f,
        Vector3f(baseColor.x / lum, baseColor.y / lum, baseColor.z / lum),
        Vector3f(1.f, 1.f, 1.f));

    auto Cspec0 = baseColor;
    // Lerp(metalness, SchlickR0FromEta(eta) * Lerp(specTint, make_float3(1.),
    // Ctint), baseColor);

    auto F = DisneyFresnel(
        Cspec0, 1.f, 0.f, dot(wi, Faceforward(wh, Vector3f(0.f, 0.f, 1.f))));

    auto ret = lum * Microfacet_G(wo, wi, micro_para) * Lum(F) /
               (4.f * cosThetaI * cosThetaO);
    std::cout << "bsdf fline" << ret << std::endl;
    return ret;
}

inline Vector2f ShadeLineElementAB(
    LineDrFloat& line,
    PatchDrFloat& patch,
    float glints_roughness,
    float line_width)
{
    Vector3f camera_pos_uv = patch.camera_pos_uv;
    Vector3f light_pos_uv = patch.light_pos_uv;

    // std::cout << camera_pos_uv << std::endl;

    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    auto center = (p0 + p1 + p2 + p3) / 4.0f;

    auto p = Vector3f(center.x, center.y, 0.f);

    Vector3f camera_dir = normalize(camera_pos_uv - p);

    Vector3f light_dir = normalize(light_pos_uv - p);

    Vector2f cam_dir_2D = Vector2f(camera_dir.x, camera_dir.y);
    Vector2f light_dir_2D = Vector2f(light_dir.x, light_dir.y);

    auto line_direction = normalize(line.end_point - line.begin_point);

    auto local_cam_dir = Vector3f(
        cross_2d(cam_dir_2D, line_direction),
        dot(cam_dir_2D, line_direction),
        camera_dir.z);

    auto local_light_dir = Vector3f(
        cross_2d(light_dir_2D, line_direction),
        dot(light_dir_2D, line_direction),
        light_dir.z);

    auto half_vec = normalize((local_cam_dir + local_light_dir));

    Vector2f points[4];

    points[0] = signed_areaAB(line, p0);
    points[1] = signed_areaAB(line, p1);
    points[2] = signed_areaAB(line, p2);
    points[3] = signed_areaAB(line, p3);

    auto minimum = std::min(
        std::min(std::min(points[0].x, points[1].x), points[2].x), points[3].x);
    auto maximum = std::max(
        std::max(std::max(points[0].x, points[1].x), points[2].x), points[3].x);

    Float left_cut = -line_width;
    Float right_cut = line_width;

    Float a[4];
    Float b[4];
    Float lower[4];
    Float upper[4];

    a[0] = slope(points[0], points[1]);
    a[1] = slope(points[1], points[2]);
    a[2] = slope(points[2], points[3]);
    a[3] = slope(points[3], points[0]);

    for (int i = 0; i < 4; ++i) {
        std::cout << a[i] << " ";
    }
    std::cout << std::endl;

    b[0] = intercept(points[0], points[1]);
    b[1] = intercept(points[1], points[2]);
    b[2] = intercept(points[2], points[3]);
    b[3] = intercept(points[3], points[0]);

    upper[3] = points[0].x;
    upper[0] = points[1].x;
    upper[1] = points[2].x;
    upper[2] = points[3].x;

    lower[0] = points[0].x;
    lower[1] = points[1].x;
    lower[2] = points[2].x;
    lower[3] = points[3].x;

    for (int i = 0; i < 4; i++) {
        upper[i] = select(upper[i] >= right_cut, right_cut, upper[i]);
        upper[i] = select(upper[i] <= left_cut, left_cut, upper[i]);

        lower[i] = select(lower[i] >= right_cut, right_cut, lower[i]);
        lower[i] = select(lower[i] <= left_cut, left_cut, lower[i]);
    }

    std::cout.precision(10);

    std::cout << "uppers: ";

    for (int i = 0; i < 4; ++i) {
        std::cout << upper[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "lowers: ";

    for (int i = 0; i < 4; ++i) {
        std::cout << lower[i] << " ";
    }
    std::cout << std::endl;

    auto temp = lineShadeAB(
                    lower,
                    upper,
                    a,
                    b,
                    sqrt(Float(glints_roughness)),
                    half_vec.x,
                    half_vec.z,
                    line_width) /
                length(light_pos_uv - p) /
                length(light_pos_uv - p) /* * Float(params.exposure)*/ *
                bsdf_f_line(camera_dir, light_dir, Float(glints_roughness));

    std::cout << temp << std::endl;

    auto patch_area = abs(cross_2d(p1 - p0, p2 - p0) / 2.f) +
                      abs(cross_2d(p2 - p0, p3 - p0) / 2.f);

    bool mask = minimum * maximum > 0 &&
                (abs(minimum) > line_width && abs(maximum) > line_width);

    auto result = select(mask, 0.f, abs(temp) / patch_area);

    auto glints_area = areaCalc(lower, upper, a, b);

    Vector2f result2f(result, glints_area);

    return result2f;
}
