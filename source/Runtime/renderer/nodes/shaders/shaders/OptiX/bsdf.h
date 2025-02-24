#pragma once

#include "PathTracer/Intersection.h"
#include "OptikaUtils.h"
#include "random.h"
#include "vec_math.h"

//#ifdef __CUDACC__
//#define cos __cosf
//#define sin __sinf
//#define sqrt __fsqrt_rn
//#endif

#include <cuda_runtime_api.h>
#include <cuda_runtime.h>


__device__ OPTIKA_INLINE float FrDielectric(float cosThetaI, float etaI, float etaT)
{
    // TODO: Try if shilick is faster. How much.

    cosThetaI = Clamp(cosThetaI, -1, 1);
    // Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering)
    {
        float temp = etaI;
        etaI = etaT;
        etaT = temp;
        cosThetaI = abs(cosThetaI);
    }

    // Compute _cosThetaT_ using Snell's law
    float sinThetaI = sqrt(max((float)0, 1 - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1)
        return 1;
    float cosThetaT = sqrt(max((float)0, 1 - sinThetaT * sinThetaT));
    float Rparl =
        ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp =
        ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;
}

OPTIKA_INLINE __device__ bool
// wi and n on the same side, eta is etaI/etaT
Refract(const float3& wi, const float3& n, float eta, float3& wt)
{
    // Compute $\cos \theta_\roman{t}$ using Snell's law
    float cosThetaI = dot(n, wi);
    float sin2ThetaI = max(float(0), float(1 - cosThetaI * cosThetaI));
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1)
        return false;
    float cosThetaT = sqrt(1 - sin2ThetaT);
    wt = eta * -wi + (eta * cosThetaI - cosThetaT) * float3(n);
    wt = normalize(wt);
    return true;
}

struct microfacet_params
{
    float2 alpha;
};

// Normal Distribution
OPTIKA_INLINE __device__ float MicrofacetDistribution(
    const float3& wh,
    const microfacet_params& param)
{
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta))
        return 0.;
    const float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    float e = (Cos2Phi(wh) / (param.alpha.x * param.alpha.x) +
               Sin2Phi(wh) / (param.alpha.y * param.alpha.y)) *
              tan2Theta;
    return 1 / (Pi * param.alpha.x * param.alpha.y * cos4Theta * (1 + e) * (1 + e));
}

OPTIKA_INLINE __device__ void
TrowbridgeReitzSample11(float cosTheta, float U1, float U2, float* slope_x, float* slope_y)
{
    // special case (normal incidence)
    if (cosTheta > .9999)
    {
        float r = sqrt(U1 / (1 - U1));
        float phi = 6.28318530718 * U2;
        *slope_x = r * cos(phi);
        *slope_y = r * sin(phi);
        return;
    }

    float sinTheta = sqrt(max((float)0, (float)1 - cosTheta * cosTheta));
    float tanTheta = sinTheta / cosTheta;
    float a = 1 / tanTheta;
    float G1 = 2 / (1 + sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    float A = 2 * U1 / G1 - 1;
    float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10)
        tmp = 1e10;
    float B = tanTheta;
    float D = sqrt(max(float(B * B * tmp * tmp - (A * A - B * B) * tmp), float(0)));
    float slope_x_1 = B * tmp - D;
    float slope_x_2 = B * tmp + D;
    *slope_x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    float S;
    if (U2 > 0.5f)
    {
        S = 1.f;
        U2 = 2.f * (U2 - .5f);
    }
    else
    {
        S = -1.f;
        U2 = 2.f * (.5f - U2);
    }
    float z = (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
              (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    *slope_y = S * z * sqrt(1.f + *slope_x * *slope_x);

    // CHECK(!std::isinf(*slope_y));
    // CHECK(!std::isnan(*slope_y));
}

OPTIKA_INLINE __device__ float3
TrowbridgeReitzSample(const float3& wi, const microfacet_params& param, float U1, float U2)
{
    // 1. stretch wi
    float3 wiStretched = normalize(make_float3(param.alpha.x * wi.x, param.alpha.y * wi.y, wi.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    float slope_x, slope_y;
    TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

    // 3. rotate
    float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
    slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = param.alpha.x * slope_x;
    slope_y = param.alpha.y * slope_y;

    // 5. compute normal
    return normalize(make_float3(-slope_x, -slope_y, 1.));
}

OPTIKA_INLINE __device__ float3
MicrofacetSample_wh(const float3& wo, const microfacet_params& param, unsigned& random_seed)
{
    float3 wh;
    bool flip = wo.z < 0;
    wh = TrowbridgeReitzSample(flip ? -wo : wo, param, rnd(random_seed), rnd(random_seed));
    if (flip)
        wh = -wh;
    return wh;
}

OPTIKA_INLINE __device__ float Microfacet_G1(const float3& w, microfacet_params param)
{
    float absTanTheta = abs(TanTheta(w));
    if (isinf(absTanTheta))
        return 0.f;
    // Compute _alpha_ for direction _w_
    float alpha = sqrt(
        Cos2Phi(w) * param.alpha.x * param.alpha.x + Sin2Phi(w) * param.alpha.y * param.alpha.y);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    float lambda = (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2.f;
    return 1.f / (1.f + lambda);
}

OPTIKA_INLINE __device__ float
Microfacet_G(const float3& wi, const float3& wo, microfacet_params param)
{
    return Microfacet_G1(wi, param) * Microfacet_G1(wo, param);
}

OPTIKA_INLINE __device__ float3x3 GetTBNMatrix(const float3& n, const float3& t)
{
    float3 tangent = normalize(t - dot(n, t) * n);
    float3 b = normalize(cross(n, tangent));

    return float3x3(tangent, b, n);
}

OPTIKA_INLINE __device__ void Object2Tangent(float3& v, const float3x3& TBN)
{
    float3x3 O2TMatrix;
    Inverse3x3Matrix(TBN, O2TMatrix);
    v = Mul(O2TMatrix, v);
}

OPTIKA_INLINE __device__ void Tangent2Object(float3& v, const float3x3& TBN)
{
    v = Mul(TBN, v);
}

//OPTIKA_INLINE __device__ float Lerp(float t, float s1, float s2)
//{
//    return (1 - t) * s1 + t * s2;
//}

template<typename T>
OPTIKA_INLINE __host__ __device__ T Lerp(float t, const T& s1, const T& s2)
{
    return (1 - t) * s1 + t * s2;
}

template<typename T>
OPTIKA_INLINE __host__ __device__ T lerp(float t, const T& s1, const T& s2)
{
    return (1 - t) * s1 + t * s2;
}

OPTIKA_INLINE __device__ float SchlickWeight(float cosTheta)
{
    float m = Clamp(1 - cosTheta, 0, 1);
    return (m * m) * (m * m) * m;
}

__device__ inline float FrSchlick(float R0, float cosTheta)
{
    return Lerp(SchlickWeight(cosTheta), R0, 1.0f);
}

__device__ inline float3 FrSchlick(const float3& R0, float cosTheta)
{
    return Lerp(SchlickWeight(cosTheta), R0, make_float3(1.));
}

__device__ inline float Lum(float3 color)
{
    const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
    return YWeight[0] * color.x + YWeight[1] * color.y + YWeight[2] * color.z;
}

// For a dielectric, R(0) = (eta - 1)^2 / (eta + 1)^2, assuming we're
// coming from air..
OPTIKA_INLINE __device__ float SchlickR0FromEta(float eta)
{
    return (eta - 1) * (eta - 1) / ((eta + 1) * (eta + 1));
}

OPTIKA_INLINE __device__ bool FrontFace(const Intersection& isect)
{
#ifdef __CUDACC__
    return isect.hitkind == OPTIX_HIT_KIND_TRIANGLE_FRONT_FACE ||
           isect.hitkind == OPTIX_HIT_KIND_PROCEDUAL_FRONT_FACE;
#endif
}

constexpr float bias_epsilon = 1E-3f;
struct BSDF
{
    float3 baseColor;
    float3 emissive;
    float metalness;
    float eta;
    float roughness;
    float specTrans;
    float diffTrans;
    float specTint;
    float sheen;
    float sheenTint;
    float3 shadingNormal;
    microfacet_params micro_para;

    __device__
    BSDF(float3 basecolor = make_float3(1.0),
     float3 emissive = make_float3(0.0),
     float metal = 0,
     float eta = 1,
     float rough = 1,
     float spectrans = 0,
     float difftrans = 0,
     float speculartint = 0,
     float anisotropic = 0,
     float sheen = 0,
     float sheentint = 0.5,
     float3 normalmap = make_float3(0.5,0.5,1))
        : baseColor(basecolor),
          emissive(emissive),
          metalness(metal),
          eta(eta),
          roughness(rough),
          specTrans(spectrans),
          diffTrans(difftrans),
          specTint(speculartint),
          sheen(sheen),
          sheenTint(sheentint)
    {
        shadingNormal = normalize(normalmap * 2.0 - make_float3(1));
        float aspect = sqrt(1 - anisotropic * .9);
        float ax = max(float(.001), sqrt(roughness) / aspect);
        float ay = max(float(.001), sqrt(roughness) * aspect);
        micro_para = { make_float2(ax, ay) };
    }
    __device__
    BSDF(const BSDF& bsdf) = default;
    __device__
    BSDF& operator=(const BSDF& bsdf) = default;

    __device__ float3 f(const Intersection& isect, const RayDesc& ray_out) const
    {
        RayDesc ray_in = isect.path_state_in.ray;

        float3 t = isect.tangent;
        if (length(isect.tangent) < 0.1)
        {
            t = make_float3(1, 0, 0);
            if (abs(dot(t, isect.normal)) > 0.999)
                t = make_float3(0, 1, 0);
        }
        float3x3 TBN = GetTBNMatrix(isect.normal, t);

        // if exist normal map
        if (shadingNormal.z < 0.99999)
        {
            float3x3 transform = GetTBNMatrix(shadingNormal, make_float3(1, 0, 0));
            TBN = Mul(TBN,transform);
        }

        float3 wo = ray_in.Direction, wi = ray_out.Direction;
        wo = -normalize(wo);
        Object2Tangent(wo, TBN);
        Object2Tangent(wi, TBN);
 

        float strans = specTrans;
        float metallicWeight = metalness;

        float diffuseWeight = (1 - metallicWeight) * (1 - strans);

        float3 scattered_color = make_float3(0);
        bool reflect = wi.z * wo.z > 0;
        if (reflect)
        {
            scattered_color +=
                diffuseWeight * (diffuse_f(wo, wi) + diffuse_Retro_f(wo, wi) + Sheen_f(wo, wi));
            scattered_color += MicrofacetReflect_f(wo, wi);
        }
        else
        {
            scattered_color += strans * MicrofacetTransmission_f(wo, wi);
        }

        scattered_color *= AbsCosTheta(wi);

        return scattered_color;
    }

    __device__ float3
    scatter(const Intersection& isect, RayDesc& ray_out, float& pdf, unsigned int& random_seed)
    {
        // compute wo in tangent space
        RayDesc ray_in = isect.path_state_in.ray;

        float3 t = isect.tangent;
        //float3 n = dot(isect.normal, ray_in.Direction) < 0 ? isect.normal : -isect.normal;
        if (length(isect.tangent) < 0.00001)
        {
            t = make_float3(1, 0, 0);
            if (abs(dot(t, isect.normal)) > 0.999)
                t = make_float3(0, 1, 0);
        }
        //float3 n = isect.normal;
        float3x3 TBN = GetTBNMatrix(isect.normal, t);

        // if exist normal map
        if (shadingNormal.z<0.99999)
        {
            float3x3 transform = GetTBNMatrix(shadingNormal, make_float3(1,0,0));
            TBN = Mul(transform, TBN);
        }

        float3 wo = ray_in.Direction, wi = wo;
        wo = -normalize(wo);
        Object2Tangent(wo, TBN);        

        // sample and compute coeffeients
        float strans = specTrans;
        float metallicWeight = metalness;
        float diffuseWeight = (1 - metallicWeight) * (1 - strans);

        float matchingComps = strans > 0 ? 3 : 2;
        // sample direction wi
        int bxdf_selection = int(rnd(random_seed) * matchingComps);


        bool refract = false;
        switch (bxdf_selection)
        {
            case 0: diffuse_Sample_f(wo, wi, random_seed);
                break;
            case 1: MicrofacetReflect_Samplef(wo, wi, random_seed);

                break;
            case 2: refract = MicrofacetTransmission_Samplef(wo, wi, random_seed);
                break;
            default: diffuse_Sample_f(wo, wi, random_seed);
        }

        // compute scattered color
        float3 scattered_color = make_float3(0);
        bool reflect = wi.z * wo.z > 0;
        if (reflect)
        {
            scattered_color +=
                diffuseWeight * (diffuse_f(wo, wi) + diffuse_Retro_f(wo, wi) + Sheen_f(wo, wi));
            scattered_color += MicrofacetReflect_f(wo, wi);
        }
        else
        {
            if (refract)
            {
                scattered_color += strans * MicrofacetTransmission_f(wo, wi,strans>0);
            }
        }

        // compute pdf
        pdf = default_pdf(wo, wi) + MicrofacetReflect_PDf(wo, wi) + MicrofacetTransmission_Pdf(wo, wi);
        pdf /= matchingComps;

        //scattered_color*= AbsCosTheta(wi);

        Tangent2Object(wi, TBN);
        ray_out = RayDesc(isect.position, wi);
        if (dot(wi, isect.normal) < 0)
        {
            ray_out.Origin -= (bias_epsilon * isect.normal);
        }
        else
            ray_out.Origin += (bias_epsilon * isect.normal);


        return scattered_color;
    }



    OPTIKA_INLINE __device__ float default_pdf(const float3& wo, const float3& wi) const
    {
        // return AbsCosTheta(wi) * InvPi;
        return SameHemisphere(wo, wi) ? AbsCosTheta(normalize(wi)) * InvPi : 0;
    }

    // Both wi,wo should be normalized.
    OPTIKA_INLINE __device__ float3 diffuse_f(const float3& wo, const float3& wi) const
    {
        float Fo = SchlickWeight(AbsCosTheta(wo)), Fi = SchlickWeight(AbsCosTheta(wi));
        // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing.
        // Burley 2015, eq (4).
        return baseColor * InvPi * (1 - Fo / 2) * (1 - Fi / 2);
    }

    OPTIKA_INLINE __device__ void
    diffuse_Sample_f(const float3& wo, float3& wi, unsigned& random_seed) const
    {
        wi = CosineSampleHemisphere(rnd(random_seed), rnd(random_seed));
    }

    OPTIKA_INLINE __device__ float3 diffuse_Retro_f(const float3& wo, const float3& wi) const
    {
        float3 wh = wi + wo;
        if (wh.x == 0 && wh.y == 0 && wh.z == 0)
            return make_float3(0.);
        wh = normalize(wh);
        float cosThetaD = dot(wi, wh);

        float Fo = SchlickWeight(AbsCosTheta(wo)), Fi = SchlickWeight(AbsCosTheta(wi));
        float Rr = 2 * roughness * cosThetaD * cosThetaD;

        // Burley 2015, eq (4).
        return baseColor * InvPi * Rr * (Fo + Fi + Fo * Fi * (Rr - 1));
    }

    OPTIKA_INLINE __device__ float3 Sheen_f(const float3& wo, const float3& wi) const
    {
        if (sheen == 0)
            return make_float3(0);

        float3 wh = wi + wo;
        if (wh.x == 0 && wh.y == 0 && wh.z == 0)
            return make_float3(0.);
        wh = normalize(wh);
        float cosThetaD = dot(wi, wh);

        float lum = Lum(baseColor);
        float3 Ctint = lum > 0 ? (baseColor / lum) : make_float3(1.);
        float3 Csheen = Lerp(sheenTint, make_float3(1.), Ctint);
        float3 R = sheen * Csheen;
        return R * SchlickWeight(cosThetaD) / cosThetaD;
    }

    OPTIKA_INLINE __device__ float3
    DisneyFresnel(const float3& R0, float metallic, float eta, float cosI) const
    {
        return Lerp(metallic, make_float3(FrDielectric(cosI, 1, eta)), FrSchlick(R0, cosI));
    }



    OPTIKA_INLINE __device__ float Microfacet_PDf(const float3& wo, const float3& wh) const
    {
        return MicrofacetDistribution(wh, micro_para) * Microfacet_G1(wo, micro_para) *
               abs(dot(wo, wh)) / AbsCosTheta(wo);
    }

    OPTIKA_INLINE __device__ float MicrofacetReflect_PDf(const float3& wo, const float3& wi)
        const
    {
        if (!SameHemisphere(wo, wi))
            return 0;
        float3 wh = normalize(wi + wo);
        return Microfacet_PDf(wo, wh) / (4 * dot(wo, wh));
    }

    OPTIKA_INLINE __device__ float MicrofacetTransmission_Pdf(
        const float3& wo,
        const float3& wi) const
    {
        if (SameHemisphere(wo, wi))
            return 0;
        // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
        float Eta = CosTheta(wo) > 0 ? (eta / 1) : (1 / eta);
        float3 wh = normalize(wo + wi * Eta);

        if (dot(wo, wh) * dot(wi, wh) > 0)
            return 0;

        // Compute change of variables _dwh\_dwi_ for microfacet transmission
        float sqrtDenom = dot(wo, wh) + Eta * dot(wi, wh);
        float dwh_dwi = abs((Eta * Eta * dot(wi, wh)) / (sqrtDenom * sqrtDenom));
        return Microfacet_PDf(wo, wh) * dwh_dwi;
    }

    OPTIKA_INLINE __device__ float3
    MicrofacetReflect_f(const float3& wo, const float3& wi) const
    {
        float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
        float3 wh = wi + wo;
        // Handle degenerate cases for microfacet reflection
        if (cosThetaI == 0 || cosThetaO == 0)
            return make_float3(0.);
        if (wh.x == 0 && wh.y == 0 && wh.z == 0)
            return make_float3(0.);
        wh = normalize(wh);
        // For the Fresnel call, make sure that wh is in the same hemisphere
        // as the surface normal, so that TIR is handled correctly.
        float lum = Lum(baseColor);
        // normalize lum. to isolate hue+sat
        float3 Ctint = lum > 0 ? (baseColor / lum) : make_float3(1.);
        float3 Cspec0 = Lerp(
            metalness, SchlickR0FromEta(eta) * Lerp(specTint, make_float3(1.), Ctint), baseColor);
        float3 F =
            DisneyFresnel(Cspec0, metalness, eta, dot(wi, Faceforward(wh, make_float3(0, 0, 1))));

        return make_float3(1.f) * MicrofacetDistribution(wh, micro_para) *
               Microfacet_G(wo, wi, micro_para) * F / (4 * cosThetaI * cosThetaO);
    }

    OPTIKA_INLINE __device__ void
    MicrofacetReflect_Samplef(const float3& wo, float3& wi, unsigned& random_seed) const
    {
        // Sample microfacet orientation $\wh$ and reflected direction$\wi$
        float3 wh = MicrofacetSample_wh(wo, micro_para, random_seed);
        wi = reflect(wo, wh);
    }

    OPTIKA_INLINE __device__ float3
    MicrofacetTransmission_f(const float3& wo, const float3& wi, bool log = false) const
    {
        if (SameHemisphere(wo, wi))
            return make_float3(0);  // transmission only

        float cosThetaO = CosTheta(wo);
        float cosThetaI = CosTheta(wi);
        if (cosThetaI == 0 || cosThetaO == 0)
            return make_float3(0);

        // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
        float Eta = CosTheta(wo) > 0 ? (eta / 1) : (1 / eta);
        float3 wh = normalize(wo + wi * Eta);
        
        if (wh.z < 0)
            wh = -wh;

        // Same side?
        if (dot(wo, wh) * dot(wi, wh) > 0)
            return make_float3(0);

        // float3 F = fresnel.Evaluate(dot(wo, wh));
        float3 F = make_float3(FrDielectric(dot(wo, wh), 1, eta));

        float sqrtDenom = dot(wo, wh) + Eta * dot(wi, wh);
        // float factor = (mode == TransportMode::Radiance) ? (1 / Eta) : 1;
        float factor = 1 / Eta;


        float3 T = make_float3(sqrt(baseColor.x), sqrt(baseColor.y), sqrt(baseColor.z));

        auto ret = (make_float3(1.f) - F) * T *
                   abs(MicrofacetDistribution(wh, micro_para) * Microfacet_G(wo, wi, micro_para) *
                       Eta * Eta * abs(dot(wi, wh)) * abs(dot(wo, wh)) * factor * factor /
                       (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));

        return ret;
    }

    OPTIKA_INLINE __device__ bool MicrofacetTransmission_Samplef(
        const float3& wo,
        float3& wi,
        unsigned& random_seed) const
    {
        float3 wh = MicrofacetSample_wh(wo, micro_para, random_seed);

        float Eta = CosTheta(wo) > 0 ? (1 / eta) : (eta / 1);
        return Refract(wo, wh, Eta, wi);
    }

};

#undef cos
#undef sin 
#undef sqrt