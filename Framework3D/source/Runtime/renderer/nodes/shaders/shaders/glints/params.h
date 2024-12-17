#pragma once

#ifndef __CUDACC__
#include "RHI/internal/optix/WorkQueue.cuh"
#else
#include "WorkQueue.cuh"
#endif

struct Ray {
    float3 origin;
    float3 direction;
    float tmin;
    float tmax;
};

struct Patch {
    float2 uv0, uv1, uv2, uv3;
    float3 camera_pos_uv;
    float3 light_pos_uv;
};

struct MeshTracingParams {
    OptixTraversableHandle handle;
    float* vertices;
    unsigned* indices;
    Ray* rays;
    WorkQueue<Patch>* append_buffer;
    int2* pixel_targets;
};

struct GlintsTracingParams {
    OptixTraversableHandle handle;
    Patch* patches;
    WorkQueue<uint2>* patch_line_pairs;
};

extern "C" {
extern __constant__ GlintsTracingParams params;
extern __constant__ MeshTracingParams mesh_params;
}
