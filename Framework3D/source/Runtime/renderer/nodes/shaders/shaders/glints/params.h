#pragma once

#ifndef __CUDACC__
#include "RHI/internal/optix/WorkQueue.cuh"
#else
#include "WorkQueue.cuh"
#endif

struct Patch {
    float2 uv0;
    float2 uv1;
    float2 uv2;
    float2 uv3;
};

struct GlintsTracingParams {
    OptixTraversableHandle handle;
    Patch* patches;
    WorkQueue<uint2>* patch_line_pairs;
};

extern "C" {
extern __constant__ GlintsTracingParams params;
}
