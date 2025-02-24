// material_eval_sample_pdf.slang

#include "matrix_op.h"
#include "utils/HitObject.h"
#include "utils/random.slangh"
#include "utils/view_cb.h"

import Scene.SceneTypes;
import Utils.Math.MathHelpers;

RaytracingAccelerationStructure SceneBVH;

StructuredBuffer<HitObjectInfo> hitObjects;
StructuredBuffer<uint2> in_PixelTarget;

import Scene.BindlessVertexBuffer;

RWStructuredBuffer<uint2> PixelTarget;
RWStructuredBuffer<float4> Eval;
RWStructuredBuffer<RayInfo> Sample;
RWStructuredBuffer<float> Weight;
RWStructuredBuffer<float> Pdf;
RWStructuredBuffer<uint> random_seeds;

import Rendering.Materials.BSDFs.StandardBSDF;
import utils.Sampling.UniformSampleGenerator;
import Scene.Material.MaterialData;
import utils.Math.ShadingFrame;

struct Payload {
    uint seed;
};

struct Attributes {
    float2 barycentric;
};

// Ray generation shader
[shader("raygeneration")]
void RayGen()
{
    int id = DispatchRaysIndex().x;

    HitObjectInfo hitObject = hitObjects[id];

    RayDesc ray_desc = get_ray_desc(hitObject.rays);
    Payload payload;
    payload.seed = random_seeds[DispatchRaysIndex().x];

// #define USE_SER
#ifdef USE_SER
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

    object.Invoke(SceneBVH, object, payload);
#else
    // If not using SER, we launch the ray, like a normal hlsl shader
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray_desc, payload);
#endif
    random_seeds[DispatchRaysIndex().x] = payload.seed;
}

struct ShadowPayload {
    float3 shadow_mask;
};

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

    var instance_id = InstanceID();
    var prim_id = PrimitiveIndex();
    VertexData vertex = get_interpolated_vertex(instance_id, prim_id, attrib.barycentric);

    // if (dot(WorldRayDirection(), vertex.normalW) >= 0.0) {
    //     vertex.normalW = -vertex.normalW;
    // }

    // UniformSampleGenerator s;

    // result = bsdf.eval(ObjectRayDirection(), ObjectRayDirection(), s, context);

    var seed = payload.seed;

    // Calculate ambient occlusion
    ShadowPayload shadowPayload;
    shadowPayload.shadow_mask = float3(1, 1, 1);
    float pdf;
    float3 ray_dir = sample_cosine_hemisphere_concentric(random_float2(seed), pdf);

    bool valid;
    ShadingFrame sf = ShadingFrame.createSafe(vertex.normalW, vertex.tangentW, valid);
    var world_ray_dir = sf.fromLocal(ray_dir);

    RayDesc shadow_ray_desc;
    shadow_ray_desc.Origin = vertex.posW + vertex.normalW * 0.001;
    shadow_ray_desc.Direction = world_ray_dir;
    shadow_ray_desc.TMin = 0.01;
    shadow_ray_desc.TMax = 1000.0;

    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 1, 1, 1, shadow_ray_desc, shadowPayload);
    result = shadowPayload.shadow_mask;

    Eval[id] = float4(result, 1);
    // * float4(hitObjects[id].rays.throughput.toFloat3(), 1.0);

    // Acquire the material information here.

    payload.seed = seed;
}

// Miss shader
[shader("miss")]
void Miss(inout Payload payload: SV_RayPayload)
{
    uint id = DispatchRaysIndex().x;
    PixelTarget[id] = in_PixelTarget[id];
    Eval[id] = float4(1, 0, 1, 1);
}

[shader("closesthit")]
void ShadowHit(inout ShadowPayload payload: SV_RayPayload)
{
    payload.shadow_mask = float3(0, 0, 0);
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload: SV_RayPayload)
{
    payload.shadow_mask = float3(1, 1, 1);
}

