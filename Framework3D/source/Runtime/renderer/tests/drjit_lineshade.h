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
using Bool = bool;
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
    Float name##_power6 = (name) * name##_power5;

#define CalcPowerSeriesComplex(name)                \
    Complex name##_power2 = (name) * name;          \
    Complex name##_power3 = (name) * name##_power2; \
    Complex name##_power4 = (name) * name##_power3; \
    Complex name##_power5 = (name) * name##_power4; \
    Complex name##_power6 = (name) * name##_power5;

#define DeclarePowerSeries(name)                                               \
    Float name, Float name##_power2, Float name##_power3, Float name##_power4, \
        Float name##_power5, Float name##_power6

#define UsePowerSeries(name)                                          \
    name, name##_power2, name##_power3, name##_power4, name##_power5, \
        name##_power6

#undef Power

#define Power(name, n) name##_power##n

#define work_for_div 100000000.f

Complex sumpart(
    Float lower,
    Float upper,
    Complex y,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeriesComplex(y);

    auto log_val_u = log(upper - y);
    auto log_val_l = log(lower - y);

    auto a =
        -((((powf(halfZ - Power(halfZ, 3) * Power(r, 2), 2) +
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
                Power(y, 3)) *
           (log_val_u - log_val_l))) *
        work_for_div;
    auto b = (halfX * halfZ * Power(r, 2) * Power(width, 3) +
              2 * (1 + (-2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
                  Power(width, 2) * y -
              12 * halfX * halfZ * Power(r, 2) * width * Power(y, 2) -
              8 * (-1 + Power(halfZ, 2) * Power(r, 2)) * Power(y, 3)) *
             work_for_div;

    return a / b;
}

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

auto calc_res(
    Float x,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeries(x);

    auto a =
        (4 *
         (halfX * halfZ * Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) *
              (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                   Power(r, 4)) *
              Power(width, 5) -
          2 *
              (powf(-1.f + Power(halfZ, 2) * Power(r, 2), 3) *
                   (1 + Power(halfZ, 2) * Power(r, 2)) +
               4 * Power(halfX, 4) * Power(halfZ, 2) * Power(r, 6) *
                   (3 + Power(halfZ, 2) * Power(r, 2)) +
               Power(halfX, 2) * Power(r, 2) *
                   (2 - 11 * Power(halfZ, 2) * Power(r, 2) +
                    4 * Power(halfZ, 4) * Power(r, 4) +
                    5 * Power(halfZ, 6) * Power(r, 6))) *
              Power(width, 4) * x +
          4 * halfX * halfZ * Power(r, 2) *
              (6 + (-19 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               2 *
                   (6 * Power(halfX, 4) - Power(halfX, 2) * Power(halfZ, 2) -
                    4 * Power(halfZ, 4)) *
                   Power(r, 4) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                   (4 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 6)) *
              Power(width, 3) * Power(x, 2) +
          8 * (1 + Power(halfZ, 2) * Power(r, 2)) *
              (-1 + (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                   Power(r, 4)) *
              Power(width, 2) * Power(x, 3) -
          64 * halfX * halfZ * Power(r, 2) *
              (-1 + Power(halfZ, 2) * Power(r, 2)) *
              (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * width *
              Power(x, 4) -
          32 * powf(-1.f + Power(halfZ, 2) * Power(r, 2), 2) *
              (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              Power(x, 5))) *
        work_for_div;

    auto b = ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              (-((1 + halfZ * r) * Power(width, 2)) +
               4 * halfX * r * width * x + 4 * (-1 + halfZ * r) * Power(x, 2)) *
              ((1 - halfZ * r) * Power(width, 2) + 4 * halfX * r * width * x +
               4 * (1 + halfZ * r) * Power(x, 2))) *
             work_for_div;

    return a / b;
}

template<typename T>
inline T select(Bool cond, T true_, T false_)
{
    return cond ? true_ : false_;
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

    return lum * Microfacet_G(wo, wi, micro_para) * Lum(F) /
           (4.f * cosThetaI * cosThetaO);
}

Float lineShade(
    Float lower,
    Float upper,
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

    auto ret = Complex(0, 0);

    for (int i = 0; i < 4; i++) {
        auto part = sumpart(
            lower,
            upper,
            c[i],
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r));

        ret += part;
    }

    ret *= (Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) * width) *
           work_for_div /
           ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
            work_for_div);

    ret += calc_res(
               upper,
               UsePowerSeries(width),
               UsePowerSeries(halfX),
               UsePowerSeries(halfZ),
               UsePowerSeries(r)) -
           calc_res(
               lower,
               UsePowerSeries(width),
               UsePowerSeries(halfX),
               UsePowerSeries(halfZ),
               UsePowerSeries(r));
    Float coeff =
        -alpha * alpha * work_for_div /
        ((Float(8.) * M_PI * powf(-1.f + Power(halfZ, 2) * Power(r, 2), 3)) *
         work_for_div);

    ret *= coeff;

    return GetReal(ret);
}

Float cross_2d(const Vector2f& a, const Vector2f& b)
{
    return a.x * b.y - a.y * b.x;
}

Float signed_area(const LineDrFloat& line, Vector2f point)
{
    // The direction is expected to be normalized
    return cross_2d(
        point - (line.begin_point + line.end_point) / 2.f,
        normalize(-line.begin_point + line.end_point));
}

Float integral_triangle_area(
    const Vector2f& p0,
    const Vector2f& p1,
    const Vector2f& p2,
    Float t,
    const Vector2f& axis)
{
    auto result = select(
        t >= 0 && t <= dot(p1 - p0, axis),
        abs(cross_2d(
            t / dot(p2 - p0, axis) * (p2 - p0),
            t / dot(p1 - p0, axis) * (p1 - p0))) /
            2.f,
        select(
            t > dot(p1 - p0, axis) && t <= dot(p2 - p0, axis),
            abs(cross_2d((p2 - p0), (p1 - p0))) / 2.f -
                abs(cross_2d(
                    (p1 - p2) * (dot(p2 - p0, axis) - t) / dot(p1 - p2, axis),
                    (p0 - p2) * (dot(p2 - p0, axis) - t) /
                        dot(p0 - p2, axis))) /
                    2.f,
            select(
                t > dot(p2 - p0, axis),
                abs(cross_2d((p2 - p0), (p1 - p0))) / 2.f,
                0.f)));
    return result;
}

Float intersect_triangle_area(
    const Vector2f& p0,
    const Vector2f& p1,
    const Vector2f& p2,
    const LineDrFloat& line,
    Float width)
{
    Float width_half = width / 2.f;

    auto line_pos = (line.begin_point + line.end_point) / 2.f,
         line_dir = normalize(-line.begin_point + line.end_point);

    auto p0_tmp = p0, p1_tmp = p1, p2_tmp = p2;
    Vector2f vertical_dir(line_dir.y, -line_dir.x);

    p0_tmp = select(
        dot(p0 - p1, vertical_dir) >= 0 && dot(p2 - p1, vertical_dir) >= 0,
        p1,
        p0);

    p1_tmp = select(
        dot(p0 - p1, vertical_dir) >= 0 && dot(p2 - p1, vertical_dir) >= 0,
        p0,
        p1);

    auto p0_t = p0_tmp, p1_t = p1_tmp, p2_t = p2_tmp;

    p0_tmp = select(
        dot(p0_t - p2_t, vertical_dir) >= 0 &&
            dot(p1_t - p2_t, vertical_dir) >= 0,
        p2_t,
        p0_t);

    p2_tmp = select(
        dot(p0_t - p2_t, vertical_dir) >= 0 &&
            dot(p1_t - p2_t, vertical_dir) >= 0,
        p0_t,
        p2_t);

    Float x_to_vertical_dir1 = dot(p1_tmp - p0_tmp, vertical_dir);
    Float x_to_vertical_dir2 = dot(p2_tmp - p0_tmp, vertical_dir);

    auto p1_tmptmp = p1_tmp, p2_tmptmp = p2_tmp;
    p1_tmp =
        select(x_to_vertical_dir1 >= x_to_vertical_dir2, p2_tmptmp, p1_tmptmp);
    p2_tmp =
        select(x_to_vertical_dir1 >= x_to_vertical_dir2, p1_tmptmp, p2_tmptmp);

    Float t1 = dot(line_pos - p0_tmp, vertical_dir) - width_half;
    Float t2 = dot(line_pos - p0_tmp, vertical_dir) + width_half;

    return integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t2, vertical_dir) -
           integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t1, vertical_dir);
}

Float intersect_area(
    const LineDrFloat& line,
    const PatchDrFloat& patch,
    Float width)
{
    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    auto a = intersect_triangle_area(p0, p1, p2, line, width);
    auto b = intersect_triangle_area(p2, p3, p0, line, width);

    return a + b;
}

Vector2f ShadeLineElement(
    LineDrFloat& line,
    PatchDrFloat& patch,
    float glints_roughness,
    float line_width)
{
    Vector3f camera_pos_uv = patch.camera_pos_uv;
    Vector3f light_pos_uv = patch.light_pos_uv;

    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    auto center = (p0 + p1 + p2 + p3) / 4.f;

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

    auto half_vec = glm::normalize((local_cam_dir + local_light_dir));

    auto a0 = signed_area(line, p0);
    auto a1 = signed_area(line, p1);
    auto a2 = signed_area(line, p2);
    auto a3 = signed_area(line, p3);

    auto minimum = std::min(std::min(std::min(a0, a1), a2), a3);
    auto maximum = std::max(std::max(std::max(a0, a1), a2), a3);

    auto temp = lineShade(
                    std::max(minimum, -line_width),
                    std::min(maximum, line_width),
                    sqrt(Float(glints_roughness)),
                    half_vec.x,
                    half_vec.z,
                    line_width) /
                glm::length(light_pos_uv - p) / length(light_pos_uv - p) *
                bsdf_f_line(camera_dir, light_dir, Float(glints_roughness));

    auto area = intersect_area(line, patch, 2.f * line_width);

    auto patch_area = abs(cross_2d(p1 - p0, p2 - p0) / 2.f) +
                      abs(cross_2d(p2 - p0, p3 - p0) / 2.f);

    bool mask = minimum * maximum > 0 &&
                (abs(minimum) > line_width && abs(maximum) > line_width);

    auto result = select(
        mask,
        0.f,
        temp * area / patch_area /
            abs(std::max(minimum, -line_width) -
                std::min(maximum, line_width)));

    return Vector2f(result, area);
}

Float line_interand(
    Float x,
    Float alpha,
    Float ratio1,
    Float ratio2,
    Float width)
{
    return alpha * alpha / M_PI /
           powf(
               1.f + (alpha * alpha - 1.f) *
                         powf(
                             ratio1 - 4.f * x / width /
                                          (1.f - 4.f * x * x / width / width),
                             2) /
                         (1.f + ratio1 * ratio1 + ratio2 * ratio2) /
                         (1.f +
                          16.f * x * x /
                              powf(
                                  width * (1.f - 4.f * x * x / width / width),
                                  2)),
               2);
}
