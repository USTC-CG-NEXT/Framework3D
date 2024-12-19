#pragma once

#ifndef __CUDACC__
#include "RHI/internal/optix/WorkQueue.cuh"
#else
#include "WorkQueue.cuh"
#endif

#include "common.h"

struct MeshTracingParams {
    OptixTraversableHandle handle;
    float* vertices;
    unsigned* indices;
    WorkQueue<Patch>* append_buffer;
    float4x4* worldToUV;
    int2* pixel_targets;
    float4x4 worldToView;
    float4x4 viewToClip;
};

extern "C" {
extern __constant__ MeshTracingParams mesh_params;
}
