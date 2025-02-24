#pragma once

#include <drjit/loop.h>

#include <chrono>
#include <iostream>

#include "drjit/autodiff.h"
#include "drjit/jit.h"

namespace dr = drjit;

#include "Editor/Passes/GlintsTracingParams.h"
#include "Editor/Passes/drGlintsShadingPass.h"
#include "Geometry/line_segment.h"
#include "drjit/complex.h"
#include "drjit/math.h"

namespace dr = drjit;

using Float = dr::DiffArray<dr::CUDAArray<float>>;
using Complex = dr::Complex<Float>;
using Mask = dr::mask_t<Float>;
using Bool = Mask;
using LineDrFloat = LineDr<Float>;
using PatchDrFloat = PatchDr<Float>;

#define Sqrt(x)    dr::sqrt(Complex(x))
#define GetReal(x) dr::real(x)
#define Log        log


#define CalcPowerSeries(name)                   \
    Float name##_power2 = (name)*name;          \
    Float name##_power3 = (name)*name##_power2; \
    Float name##_power4 = (name)*name##_power3; \
    Float name##_power5 = (name)*name##_power4; \
    Float name##_power6 = (name)*name##_power5;

#define CalcPowerSeriesComplex(name)              \
    Complex name##_power2 = (name)*name;          \
    Complex name##_power3 = (name)*name##_power2; \
    Complex name##_power4 = (name)*name##_power3; \
    Complex name##_power5 = (name)*name##_power4; \
    Complex name##_power6 = (name)*name##_power5;

#define DeclarePowerSeries(name)                                               \
    Float name, Float name##_power2, Float name##_power3, Float name##_power4, \
        Float name##_power5, Float name##_power6

#define UsePowerSeries(name) \
    name, name##_power2, name##_power3, name##_power4, name##_power5, name##_power6

#undef Power

#define Power(name, n) name##_power##n
auto DrPi = dr::Pi<Float>;
#define work_for_div 100000000

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

    auto a = -(
        (((dr::pow(halfZ - Power(halfZ, 3) * Power(r, 2), 2) +
           Power(halfX, 2) * (-1 + Power(halfZ, 4) * Power(r, 4))) *
              Power(width, 3) -
          4 * halfX * halfZ *
              (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
              Power(width, 2) * y +
          4 *
              (-7 * Power(halfX, 2) + 7 * Power(halfZ, 2) +
               2 * (3 * Power(halfX, 4) - 2 * Power(halfX, 2) * Power(halfZ, 2) - 4 * Power(halfZ, 4)) *
                   Power(r, 2) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                   (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
              width * Power(y, 2) +
          64 * halfX * halfZ * (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              Power(y, 3)) *
         (log_val_u - log_val_l))) *
            work_for_div;
    auto b=(halfX * halfZ * Power(r, 2) * Power(width, 3) +
         2 * (1 + (-2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * Power(width, 2) * y -
         12 * halfX * halfZ * Power(r, 2) * width * Power(y, 2) -
                    8 * (-1 + Power(halfZ, 2) * Power(r, 2)) * Power(y, 3)) *
             work_for_div;


    return  a/b;
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
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
              Power(width, 5) -
          2 *
              (dr::pow(-1.f + Power(halfZ, 2) * Power(r, 2), 3) *
                   (1 + Power(halfZ, 2) * Power(r, 2)) +
               4 * Power(halfX, 4) * Power(halfZ, 2) * Power(r, 6) *
                   (3 + Power(halfZ, 2) * Power(r, 2)) +
               Power(halfX, 2) * Power(r, 2) *
                   (2 - 11 * Power(halfZ, 2) * Power(r, 2) + 4 * Power(halfZ, 4) * Power(r, 4) +
                    5 * Power(halfZ, 6) * Power(r, 6))) *
              Power(width, 4) * x +
          4 * halfX * halfZ * Power(r, 2) *
              (6 + (-19 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               2 * (6 * Power(halfX, 4) - Power(halfX, 2) * Power(halfZ, 2) - 4 * Power(halfZ, 4)) *
                   Power(r, 4) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
                   (4 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 6)) *
              Power(width, 3) * Power(x, 2) +
          8 * (1 + Power(halfZ, 2) * Power(r, 2)) *
              (-1 + (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
               Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
              Power(width, 2) * Power(x, 3) -
          64 * halfX * halfZ * Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) *
              (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * width * Power(x, 4) -
          32 * dr::pow(-1.f + Power(halfZ, 2) * Power(r, 2), 2) *
              (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * Power(x, 5))) *
        work_for_div;

    auto b = ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
              (-((1 + halfZ * r) * Power(width, 2)) + 4 * halfX * r * width * x +
               4 * (-1 + halfZ * r) * Power(x, 2)) *
              ((1 - halfZ * r) * Power(width, 2) + 4 * halfX * r * width * x +
               4 * (1 + halfZ * r) * Power(x, 2))) *
             work_for_div;

    return a / b;
}


inline Float AbsCosTheta(dr::Vector3f w)
{
    return dr::abs(w.z());
}

inline Float Lum(dr::Vector3f color)
{
    dr::Vector3f YWeight(0.212671f, 0.715160f, 0.072169f);
    return dr::dot(color, YWeight);
}

inline Float SchlickR0FromEta(Float eta)
{
    return (eta - 1) * (eta - 1) / ((eta + 1) * (eta + 1));
}

inline Float Cos2Theta(dr::Vector3f w)
{
    return w.z() * w.z();
}

inline Float Sin2Theta(dr::Vector3f w)
{
    return 1 - Cos2Theta(w);
}

inline Float Tan2Theta(dr::Vector3f w)
{
    return Sin2Theta(w) / Cos2Theta(w);
}

inline Float SinTheta(dr::Vector3f w)
{
    return sqrt(Sin2Theta(w));
}

inline Float CosTheta(dr::Vector3f w)
{
    return w.z();
}

inline Float TanTheta(dr::Vector3f w)
{
    return SinTheta(w) / CosTheta(w);
}

inline Float CosPhi(dr::Vector3f w)
{
    auto sinTheta = SinTheta(w);
    auto tmp = dr::clamp(w.x() / sinTheta, -1.f, 1.f);
    auto result = dr::select(sinTheta == 0.f, 0.f, tmp);
    return result;
}

inline Float SinPhi(dr::Vector3f w)
{
    auto sinTheta = SinTheta(w);
    auto tmp = dr::clamp(w.y() / sinTheta, -1.f, 1.f);
    auto result = dr::select(sinTheta == 0.f, 0.f, tmp);
    return result;
}

inline Float Cos2Phi(dr::Vector3f w)
{
    return CosPhi(w) * CosPhi(w);
}

inline Float Sin2Phi(dr::Vector3f w)
{
    return SinPhi(w) * SinPhi(w);
}

inline Float SchlickWeight(Float cosTheta)
{
    Float m = dr::clamp(1.f - cosTheta, 0.f, 1.f);
    return (m * m) * (m * m) * m;
}

inline dr::Vector3f FrSchlick(dr::Vector3f R0, Float cosTheta)
{
    return dr::lerp(R0, dr::Vector3f(1.f, 1.f, 1.f), SchlickWeight(cosTheta));
}

inline dr::Vector3f DisneyFresnel(dr::Vector3f R0, Float metallic, Float eta, Float cosI)
{
    return FrSchlick(R0, cosI);
}

inline dr::Vector3f Faceforward(dr::Vector3f v1, dr::Vector3f v2)
{
    auto tmp = dr::dot(v1, v2);
    auto result = dr::select(tmp < 0.f, -v1, v1);
    return result;
}

inline Float Microfacet_G1(dr::Vector3f w, dr::Vector2f param)
{
    Float absTanTheta = abs(TanTheta(w));
    auto alpha = dr::sqrt(Cos2Phi(w) * param.x() * param.x() + Sin2Phi(w) * param.y() * param.y());
    Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    Float lambda = (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2.f;
    return 1.f / (1.f + lambda);
}

inline Float Microfacet_G(dr::Vector3f wi, dr::Vector3f wo, dr::Vector2f param)
{
    return Microfacet_G1(wi, param) * Microfacet_G1(wo, param);
}

inline dr::Vector2f MakeMicroPara(Float roughness)
{
    Float ax = dr::maximum(0.001f, dr::sqrt(roughness));
    Float ay = dr::maximum(0.001f, dr::sqrt(roughness));
    dr::Vector2f micro_para(ax, ay);

    return micro_para;
}

inline Float MicrofacetDistribution(dr::Vector3f wh, dr::Vector2f param)
{
    Float tan2Theta = Tan2Theta(wh);
    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    Float e =
        (Cos2Phi(wh) / (param.x() * param.x()) + Sin2Phi(wh) / (param.y() * param.y())) * tan2Theta;
    return 1 / (DrPi * param.x() * param.y() * cos4Theta * (1 + e) * (1 + e));
}

inline Float bsdf_f(
    dr::Vector3f ray_in_d,
    dr::Vector3f ray_out_d,
    Float roughness,
    dr::Vector3f baseColor = dr::Vector3f(1.f))
{
    dr::Vector2f micro_para = MakeMicroPara(roughness);

    dr::Vector3f wo = dr::normalize(ray_in_d), wi = dr::normalize(ray_out_d);

    wo = dr::select(wo.z() < 0.f, wo * -1.f, wo);
    wi = dr::select(wi.z() < 0.f, wi * -1.f, wi);

    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    dr::Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0.f || cosThetaO == 0.f)
        return 0.f;
    // return make_float3(0.);
    if (wh.x() == 0 && wh.y() == 0 && wh.z() == 0)
        return 0.f;
    // return make_float3(0.);

    wh = dr::normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    Float lum = Lum(baseColor);

    // normalize lum. to isolate hue+sat
    auto Ctint = dr::select(
        lum > 0.f,
        dr::Vector3f(baseColor.x() / lum, baseColor.y() / lum, baseColor.z() / lum),
        dr::Vector3f(1.f, 1.f, 1.f));

    auto Cspec0 = baseColor;
    // Lerp(metalness, SchlickR0FromEta(eta) * Lerp(specTint, make_float3(1.), Ctint), baseColor);

    auto F =
        DisneyFresnel(Cspec0, 1.f, 0.f, dr::dot(wi, Faceforward(wh, dr::Vector3f(0.f, 0.f, 1.f))));

    return lum * MicrofacetDistribution(wh, micro_para) * Microfacet_G(wo, wi, micro_para) *
           Lum(F) / (4.f * cosThetaI * cosThetaO);
}

inline Float bsdf_f_line(
    dr::Vector3f ray_in_d,
    dr::Vector3f ray_out_d,
    Float roughness,
    dr::Vector3f baseColor = dr::Vector3f(1.f))
{
    dr::Vector2f micro_para = MakeMicroPara(roughness);

    dr::Vector3f wo = dr::normalize(ray_in_d), wi = dr::normalize(ray_out_d);

    wo = dr::select(wo.z() < 0.f, wo * -1.f, wo);
    wi = dr::select(wi.z() < 0.f, wi * -1.f, wi);

    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    dr::Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0.f || cosThetaO == 0.f)
        return 0.f;
    // return make_float3(0.);
    if (wh.x() == 0 && wh.y() == 0 && wh.z() == 0)
        return 0.f;
    // return make_float3(0.);

    wh = dr::normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    Float lum = Lum(baseColor);

    // normalize lum. to isolate hue+sat
    auto Ctint = dr::select(
        lum > 0.f,
        dr::Vector3f(baseColor.x() / lum, baseColor.y() / lum, baseColor.z() / lum),
        dr::Vector3f(1.f, 1.f, 1.f));

    auto Cspec0 = baseColor;
    // Lerp(metalness, SchlickR0FromEta(eta) * Lerp(specTint, make_float3(1.), Ctint), baseColor);

    auto F =
        DisneyFresnel(Cspec0, 1.f, 0.f, dr::dot(wi, Faceforward(wh, dr::Vector3f(0.f, 0.f, 1.f))));

    return lum * Microfacet_G(wo, wi, micro_para) *
           Lum(F) / (4.f * cosThetaI * cosThetaO);
}


Float lineShade(Float lower, Float upper, Float alpha, Float halfX, Float halfZ, Float width)
{
    Float r = dr::sqrt(1 - alpha * alpha);

    CalcPowerSeries(width);
    CalcPowerSeries(halfX);
    CalcPowerSeries(halfZ);
    CalcPowerSeries(r);

    Complex temp = Sqrt(
        -Power(width, 2) + Power(halfX, 2) * Power(r, 2) * Power(width, 2) +
        Power(halfZ, 2) * Power(r, 2) * Power(width, 2));

    Complex c[] = { (-(halfX * r * width) - temp) / (Float(2.) * (-1 + halfZ * r)),
                    (-(halfX * r * width) - temp) / (Float(2.) * (1 + halfZ * r)),
                    (-(halfX * r * width) + temp) / (Float(2.) * (-1 + halfZ * r)),
                    (-(halfX * r * width) + temp) / (Float(2.) * (1 + halfZ * r)) };

    auto ret = Complex(0, 0);

    for (int i = 0; i < 4; i++)
    {
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

    ret *= (Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) * width)*work_for_div /
           ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2))*work_for_div);

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
    Float coeff = -alpha * alpha*work_for_div / ((Float(8.) * DrPi * dr::pow(-1.f + Power(halfZ, 2) * Power(r, 2), 3))*work_for_div);

    ret *= coeff;

    return GetReal(ret);
}


using dr::Vector2f;
using dr::Vector3f;

Float drjit_cross(const Vector2f& a, const Vector2f& b)
{
    return a.x() * b.y() - a.y() * b.x();
}

Float signed_area(const LineDrFloat& line, Vector2f point)
{
    // The direction is expected to be normalized
#ifdef TWOPOINTS
    return drjit_cross(
        point - (line.begin_point + line.end_point) / 2.f,
        dr::normalize(-line.begin_point + line.end_point));
#elif defined(POINTDIR)
    auto line_direction = dr::Vector2f(dr::cos(line.theta), dr::sin(line.theta));
    auto line_center = line.begin_point + line.length / 2.f * line_direction;
    return drjit_cross(point - line_center, line_direction);
#else
    return drjit_cross(point - line.position, line.direction);
#endif
}

Float integral_triangle_area(
    const Vector2f& p0,
    const Vector2f& p1,
    const Vector2f& p2,
    Float t,
    const Vector2f& axis)
{
    auto result = dr::select(
        t >= 0 && t <= dr::dot(p1 - p0, axis),
        dr::abs(drjit_cross(
            t / dr::dot(p2 - p0, axis) * (p2 - p0), t / dr::dot(p1 - p0, axis) * (p1 - p0))) /
            2.f,
        dr::select(
            t > dr::dot(p1 - p0, axis) && t <= dr::dot(p2 - p0, axis),
            dr::abs(drjit_cross((p2 - p0), (p1 - p0))) / 2.f -
                dr::abs(drjit_cross(
                    (p1 - p2) * (dr::dot(p2 - p0, axis) - t) / dr::dot(p1 - p2, axis),
                    (p0 - p2) * (dr::dot(p2 - p0, axis) - t) / dr::dot(p0 - p2, axis))) /
                    2.f,
            dr::select(
                t > dr::dot(p2 - p0, axis),
                dr::abs(drjit_cross((p2 - p0), (p1 - p0))) / 2.f,
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

#ifdef TWOPOINTS
    auto line_pos = (line.begin_point + line.end_point) / 2.f,
         line_dir = dr::normalize(-line.begin_point + line.end_point);
#elif defined(POINTDIR)
    auto line_dir = dr::Vector2f(dr::cos(line.theta), dr::sin(line.theta));
    auto line_pos = line.begin_point + line.length / 2.f * line_dir;

    
#else
    auto line_pos = line.position, line_dir = line.direction;
#endif

    auto p0_tmp = p0, p1_tmp = p1, p2_tmp = p2;
    Vector2f vertical_dir(line_dir.y(), -line_dir.x());

    p0_tmp = dr::select(
        dr::dot(p0 - p1, vertical_dir) >= 0 && dr::dot(p2 - p1, vertical_dir) >= 0, p1, p0);

    p1_tmp = dr::select(
        dr::dot(p0 - p1, vertical_dir) >= 0 && dr::dot(p2 - p1, vertical_dir) >= 0, p0, p1);

    auto p0_t = p0_tmp, p1_t = p1_tmp, p2_t = p2_tmp;

    p0_tmp = dr::select(
        dr::dot(p0_t - p2_t, vertical_dir) >= 0 && dr::dot(p1_t - p2_t, vertical_dir) >= 0,
        p2_t,
        p0_t);

    p2_tmp = dr::select(
        dr::dot(p0_t - p2_t, vertical_dir) >= 0 && dr::dot(p1_t - p2_t, vertical_dir) >= 0,
        p0_t,
        p2_t);

    Float x_to_vertical_dir1 = dr::dot(p1_tmp - p0_tmp, vertical_dir);
    Float x_to_vertical_dir2 = dr::dot(p2_tmp - p0_tmp, vertical_dir);

    auto p1_tmptmp = p1_tmp, p2_tmptmp = p2_tmp;
    p1_tmp = dr::select(x_to_vertical_dir1 >= x_to_vertical_dir2, p2_tmptmp, p1_tmptmp);
    p2_tmp = dr::select(x_to_vertical_dir1 >= x_to_vertical_dir2, p1_tmptmp, p2_tmptmp);

    Float t1 = dr::dot(line_pos - p0_tmp, vertical_dir) - width_half;
    Float t2 = dr::dot(line_pos - p0_tmp, vertical_dir) + width_half;

    auto result = integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t2, vertical_dir);

    return integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t2, vertical_dir) -
           integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t1, vertical_dir);
}

Float intersect_area(const LineDrFloat& line, const PatchDrFloat& patch, Float width)
{
    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    return intersect_triangle_area(p0, p1, p2, line, width) +
           intersect_triangle_area(p2, p3, p0, line, width);
}

Vector2f ShadeLineElement(LineDrFloat& line, PatchDrFloat& patch, GlintsTracingParams params)
{
    Vector3f camera_pos_uv = patch.camera_pos_uv;
    Vector3f light_pos_uv = patch.light_pos_uv;

    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    // std::cout << "camera_pos_uv: " << camera_pos_uv << std::endl;
    // std::cout << "p0: " << p0 << std::endl;

    auto center = (p0 + p1 + p2 + p3) / 4.f;

    auto p = Vector3f(center.x(), center.y(), 0.f);

    Vector3f camera_dir = dr::normalize(camera_pos_uv - p);

    Vector3f light_dir = dr::normalize(light_pos_uv - p);

    Vector2f cam_dir_2D = Vector2f(camera_dir.x(), camera_dir.y());
    Vector2f light_dir_2D = Vector2f(light_dir.x(), light_dir.y());

#ifdef TWOPOINTS
    auto line_direction = dr::normalize(line.end_point - line.begin_point);

#elif defined(POINTDIR)
    auto line_direction = dr::Vector2f(dr::cos(line.theta), dr::sin(line.theta));
#else
    auto line_direction = line.direction;
#endif

    auto local_cam_dir = Vector3f(
        drjit_cross(cam_dir_2D, line_direction), dr::dot(cam_dir_2D, line_direction),
        camera_dir.z());

    auto local_light_dir = Vector3f(
        drjit_cross(light_dir_2D, line_direction),
        dr::dot(light_dir_2D, line_direction),
        light_dir.z());

    auto half_vec = dr::normalize((local_cam_dir + local_light_dir));


    auto a0 = signed_area(line, p0);
    auto a1 = signed_area(line, p1);
    auto a2 = signed_area(line, p2);
    auto a3 = signed_area(line, p3);

    auto minimum = dr::minimum(dr::minimum(dr::minimum(a0, a1), a2), a3);
    auto maximum = dr::maximum(dr::maximum(dr::maximum(a0, a1), a2), a3);

    Float width = 1.f / params.width;  // glints_tracing_params.width;

    Float cut = 0.4f;

    auto temp = lineShade(
                    dr::maximum(minimum, -cut * width),
                    dr::minimum(maximum, cut * width),
                    sqrt(Float(params.glints_roughness)),
                    half_vec.x(),
                    half_vec.z(),
                    width) /
                dr::norm(light_pos_uv - p) / dr::norm(light_pos_uv - p) * Float(params.exposure) *
                bsdf_f_line(camera_dir, light_dir, Float(params.glints_roughness));

    auto area = intersect_area(line, patch, 2.f * cut * width);
    auto patch_area =
        dr::abs(drjit_cross(p1 - p0, p2 - p0) / 2.f) + dr::abs(drjit_cross(p2 - p0, p3 - p0) / 2.f);

    Mask mask =
        minimum * maximum > 0 && (dr::abs(minimum) > cut * width && dr::abs(maximum) > cut * width);

    auto result = dr::select(
        mask,
        0.f,
        temp * area / patch_area /
            abs(dr::maximum(minimum, -cut * width) - dr::minimum(maximum, cut * width)));

    return Vector2f(result,area);
}

Float line_interand(Float x, Float alpha, Float ratio1, Float ratio2, Float width)
{
    return alpha * alpha / DrPi /
           dr::pow(
               1.f +
                   (alpha * alpha - 1.f) *
                       dr::pow(ratio1 - 4.f * x / width / (1.f - 4.f * x * x / width / width), 2) /
                       (1.f + ratio1 * ratio1 + ratio2 * ratio2) /
                       (1.f +
                        16.f * x * x / dr::pow(width * (1.f - 4.f * x * x / width / width), 2)),
               2);
}
