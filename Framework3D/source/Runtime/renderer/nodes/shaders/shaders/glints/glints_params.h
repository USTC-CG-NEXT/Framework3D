#pragma once

#ifndef __CUDACC__
#include "RHI/internal/optix/WorkQueue.cuh"
#else
#include "WorkQueue.cuh"
#endif
#include "common.h"

struct GlintsTracingParams {
    OptixTraversableHandle handle;
    Patch* patches;
    WorkQueue<uint2>* patch_line_pairs;
};

extern "C" {
extern __constant__ GlintsTracingParams params;
}
