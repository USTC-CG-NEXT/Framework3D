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
};

struct Corners {
    float3 v0, v1, v2;
};

struct float4x4 {
    float m[4][4];

    __device__ float4 operator*(const float4& vec) const
    {
        float4 result;
        result.x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z +
                   m[0][3] * vec.w;
        result.y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z +
                   m[1][3] * vec.w;
        result.z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z +
                   m[2][3] * vec.w;
        result.w = m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z +
                   m[3][3] * vec.w;
        return result;
    }

    __device__ float4x4 get_transposed() const
    {
        float4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[j][i];
            }
        }
        return result;
    }

    __device__ float4x4 get_inverse() const
    {
        float4x4 result;

        float inv[4][4];
        float det;
        int i;

        inv[0][0] = m[1][1] * m[2][2] * m[3][3] - m[1][1] * m[2][3] * m[3][2] -
                    m[2][1] * m[1][2] * m[3][3] + m[2][1] * m[1][3] * m[3][2] +
                    m[3][1] * m[1][2] * m[2][3] - m[3][1] * m[1][3] * m[2][2];
        inv[1][0] = -m[1][0] * m[2][2] * m[3][3] + m[1][0] * m[2][3] * m[3][2] +
                    m[2][0] * m[1][2] * m[3][3] - m[2][0] * m[1][3] * m[3][2] -
                    m[3][0] * m[1][2] * m[2][3] + m[3][0] * m[1][3] * m[2][2];
        inv[2][0] = m[1][0] * m[2][1] * m[3][3] - m[1][0] * m[2][3] * m[3][1] -
                    m[2][0] * m[1][1] * m[3][3] + m[2][0] * m[1][3] * m[3][1] +
                    m[3][0] * m[1][1] * m[2][3] - m[3][0] * m[1][3] * m[2][1];
        inv[3][0] = -m[1][0] * m[2][1] * m[3][2] + m[1][0] * m[2][2] * m[3][1] +
                    m[2][0] * m[1][1] * m[3][2] - m[2][0] * m[1][2] * m[3][1] -
                    m[3][0] * m[1][1] * m[2][2] + m[3][0] * m[1][2] * m[2][1];
        inv[0][1] = -m[0][1] * m[2][2] * m[3][3] + m[0][1] * m[2][3] * m[3][2] +
                    m[2][1] * m[0][2] * m[3][3] - m[2][1] * m[0][3] * m[3][2] -
                    m[3][1] * m[0][2] * m[2][3] + m[3][1] * m[0][3] * m[2][2];
        inv[1][1] = m[0][0] * m[2][2] * m[3][3] - m[0][0] * m[2][3] * m[3][2] -
                    m[2][0] * m[0][2] * m[3][3] + m[2][0] * m[0][3] * m[3][2] +
                    m[3][0] * m[0][2] * m[2][3] - m[3][0] * m[0][3] * m[2][2];
        inv[2][1] = -m[0][0] * m[2][1] * m[3][3] + m[0][0] * m[2][3] * m[3][1] +
                    m[2][0] * m[0][1] * m[3][3] - m[2][0] * m[0][3] * m[3][1] -
                    m[3][0] * m[0][1] * m[2][3] + m[3][0] * m[0][3] * m[2][1];
        inv[3][1] = m[0][0] * m[2][1] * m[3][2] - m[0][0] * m[2][2] * m[3][1] -
                    m[2][0] * m[0][1] * m[3][2] + m[2][0] * m[0][2] * m[3][1] +
                    m[3][0] * m[0][1] * m[2][2] - m[3][0] * m[0][2] * m[2][1];
        inv[0][2] = m[0][1] * m[1][2] * m[3][3] - m[0][1] * m[1][3] * m[3][2] -
                    m[1][1] * m[0][2] * m[3][3] + m[1][1] * m[0][3] * m[3][2] +
                    m[3][1] * m[0][2] * m[1][3] - m[3][1] * m[0][3] * m[1][2];
        inv[1][2] = -m[0][0] * m[1][2] * m[3][3] + m[0][0] * m[1][3] * m[3][2] +
                    m[1][0] * m[0][2] * m[3][3] - m[1][0] * m[0][3] * m[3][2] -
                    m[3][0] * m[0][2] * m[1][3] + m[3][0] * m[0][3] * m[1][2];
        inv[2][2] = m[0][0] * m[1][1] * m[3][3] - m[0][0] * m[1][3] * m[3][1] -
                    m[1][0] * m[0][1] * m[3][3] + m[1][0] * m[0][3] * m[3][1] +
                    m[3][0] * m[0][1] * m[1][3] - m[3][0] * m[0][3] * m[1][1];
        inv[3][2] = -m[0][0] * m[1][1] * m[3][2] + m[0][0] * m[1][2] * m[3][1] +
                    m[1][0] * m[0][1] * m[3][2] - m[1][0] * m[0][2] * m[3][1] -
                    m[3][0] * m[0][1] * m[1][2] + m[3][0] * m[0][2] * m[1][1];
        inv[0][3] = -m[0][1] * m[1][2] * m[2][3] + m[0][1] * m[1][3] * m[2][2] +
                    m[1][1] * m[0][2] * m[2][3] - m[1][1] * m[0][3] * m[2][2] -
                    m[2][1] * m[0][2] * m[1][3] + m[2][1] * m[0][3] * m[1][2];
        inv[1][3] = m[0][0] * m[1][2] * m[2][3] - m[0][0] * m[1][3] * m[2][2] -
                    m[1][0] * m[0][2] * m[2][3] + m[1][0] * m[0][3] * m[2][2] +
                    m[2][0] * m[0][2] * m[1][3] - m[2][0] * m[0][3] * m[1][2];
        inv[2][3] = -m[0][0] * m[1][1] * m[2][3] + m[0][0] * m[1][3] * m[2][1] +
                    m[1][0] * m[0][1] * m[2][3] - m[1][0] * m[0][3] * m[2][1] -
                    m[2][0] * m[0][1] * m[1][3] + m[2][0] * m[0][3] * m[1][1];
        inv[3][3] = m[0][0] * m[1][1] * m[2][2] - m[0][0] * m[1][2] * m[2][1] -
                    m[1][0] * m[0][1] * m[2][2] + m[1][0] * m[0][2] * m[2][1] +
                    m[2][0] * m[0][1] * m[1][2] - m[2][0] * m[0][2] * m[1][1];

        det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] +
              m[0][3] * inv[3][0];

        if (det == 0)
            return result;

        det = 1.0 / det;

        for (i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = inv[i][j] * det;
            }
        }

        return result;
    }

    __device__ float4x4 operator*(const float4x4& other) const
    {
        float4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
};

struct MeshTracingParams {
    OptixTraversableHandle handle;
    float* vertices;
    unsigned* indices;
    WorkQueue<Patch>* append_buffer;
    Corners* corners;
    int2* pixel_targets;
    float4x4 worldToClip;
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
