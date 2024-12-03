#pragma once

#include "WorkQueue.cuh"

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
    float width;
};

extern "C" {
extern __constant__ GlintsTracingParams params;
}
