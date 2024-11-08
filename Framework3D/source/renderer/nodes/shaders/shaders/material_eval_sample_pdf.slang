// material_eval_sample_pdf.slang

#include "matrix_op.h"
#include "utils/HitObject.h"
#include "utils/view_cb.h"
RaytracingAccelerationStructure SceneBVH : register(t0);

StructuredBuffer<HitObjectInfo> hitObjects : register(t1);
StructuredBuffer<uint2> in_PixelTarget : register(t2);

ByteAddressBuffer t_BindlessBuffers[] : register(t0, space1);

RWStructuredBuffer<uint2> PixelTarget : register(u0);
RWStructuredBuffer<float4> Eval : register(u1);
RWStructuredBuffer<RayDesc> Sample : register(u2);
RWStructuredBuffer<float> Weight : register(u3);
RWStructuredBuffer<float> Pdf : register(u4);

import Rendering.Materials.BSDFs.StandardBSDF;
import utils.Sampling.UniformSampleGenerator;
import Scene.Material.MaterialData;

struct Payload {
    int foo;
};

struct Attributes {
    float2 uv;
};

// Ray generation shader
[shader("raygeneration")]
void RayGen()
{
    int id = DispatchRaysIndex().x;

    HitObjectInfo hitObject = hitObjects[id];

    RayDesc ray_desc = get_ray_desc(hitObject.rays);

    HitObject object = HitObject.MakeHit<Attributes>(
        SceneBVH,
        hitObject.InstanceIndex,
        hitObject.GeometryIndex,
        hitObject.PrimitiveIndex,
        hitObject.HitKind,
        hitObject.RayContributionToHitGroupIndex,
        hitObject.MultiplierForGeometryContributionToHitGroupIndex,
        ray_desc,
        { hitObject.attributes });

    ReorderThread(object);

    Payload payload;
    object.Invoke(SceneBVH, object, payload);
}

// Closest hit shader
[shader("closesthit")]
void ClosestHit(
    inout Payload payload: SV_RayPayload,
    Attributes attrib: SV_IntersectionAttributes)
{
    uint id = DispatchRaysIndex().x;
    PixelTarget[id] = in_PixelTarget[id];

    BSDFContext context = BSDFContext(1.5f, 1.0f / 1.5f);

    float3 result = { 1, 0, 1 };
    StandardBSDFData data;
    data.diffuse = 1;
    data.metallic = 0;
    data.specularTransmission = 0;
    data.diffuseTransmission = 0;
    data.specular = 1;
    MaterialHeader header;
    header.setActiveLobes(uint32_t(LobeType::DiffuseReflection));
    var dir = ObjectRayDirection();

    StandardBSDF bsdf = StandardBSDF(dir, header, data);

    UniformSampleGenerator s;
    
    result = bsdf.eval(ObjectRayDirection(), ObjectRayDirection(), s, context);
    result = WorldRayDirection();

    Eval[id] = float4(result, 1);

    // Acquire the material information here.
}

// Miss shader
[shader("miss")]
void Miss(inout Payload payload: SV_RayPayload)
{
    uint id = DispatchRaysIndex().x;
    PixelTarget[id] = in_PixelTarget[id];
    Eval[id] = float4(1, 0, 1, 1);
}
