#pragma once



#include "random.h"
#include "vec_math.h"

#include "OptikaUtils.h"
#include "PathTracer/Intersection.h"
#include "bsdf.h"
#include "cuda/texture.h"

struct Material
{
    //float3 baseColor;
    Texture3f baseColor;
    Texture3f normalMap;
    float3 specularColor;
    float3 emissive;

    Texture1f metalness;
    float eta;
    Texture1f roughness;
    float specTrans;
    float diffTrans;
    float specTint;
    float anisotropic;
    float sheen;
    float sheenTint;

    // Table of BxDF selection:
    // diffuse | specular
    OPTIKA_INLINE OPTIKA_HOSTDEVICE
    Material(
        Texture3f basecolor = make_float3(1.0),
        float3 specularolor = make_float3(1.0),
        float3 emissive = make_float3(0.0),
        Texture1f metal = 0.f,
        float eta = 1.0,
        Texture1f rough = 1.0f,
        float spectrans = 0,
        float difftrans = 0,
        float speculartint = 0,
        float anisotropic = 0,
        float sheen = 0,
        float sheentint = 0.5, 
        Texture3f normalmap = make_float3(0.5f, 0.5f, 1.0f)
        )
        : baseColor(basecolor),
          specularColor(specularolor),
          emissive(emissive),
          metalness(metal),
          eta(eta),
          roughness(rough),
          specTrans(spectrans),
          diffTrans(difftrans),
          specTint(speculartint),
          anisotropic(anisotropic),
          sheen(sheen),
          sheenTint(sheentint),
          normalMap(normalmap)
    {
    }

 OPTIKA_INLINE __device__ BSDF CreatBSDF(const Intersection& isect) const
    {
        float3 c;
        baseColor.sample(&c, isect.uv);
        float3 n;
        normalMap.sample(&n, isect.uv);
        float rough;
        roughness.sample(&rough, isect.uv);
        float metal;
        metalness.sample(&metal, isect.uv);
        
        return BSDF(
            c,
            emissive,
            metal,
            eta,
            rough,
            specTrans,
            diffTrans,
            specTint,
            anisotropic,
            sheen,
            sheenTint,
            n);
        
    }
    OPTIKA_INLINE OPTIKA_HOSTDEVICE
    void operator=(const Material& material)
    {
        //printf("copy Material");
        baseColor = material.baseColor;
        specularColor = material.specularColor;
        emissive = material.emissive;
        metalness = material.metalness;
        eta = material.eta;
        roughness = material.roughness;
        specTrans = material.specTrans;
        diffTrans = material.diffTrans;
        specTint = material.specTint;
        anisotropic = material.anisotropic;
        sheen = material.sheen;
        sheenTint = material.sheenTint;
    }


    const float bias_epsilon = 2E-4f;
};

