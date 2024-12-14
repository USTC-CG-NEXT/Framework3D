#pragma once

#include "USTC_CG.h"
#define BASIC_SOCKET_TYPES Int, String, Float, Bool

#define STAGE_SOCKET_TYPES Layer, PyObj, NumpyArray, TorchTensor, SocketGroup

#if USTC_CG_WITH_TORCH
#define TORCH_TENSOR TorchTensor,
#else
#define TORCH_TENSOR
#endif

#define STAGE_SOCKET_TYPES Layer, PyObj, NumpyArray, TORCH_TENSOR SocketGroup

#define RENDER_SOCKET_TYPES \
    Lights, Camera, Texture, Meshes, Materials, AccelStruct, Buffer

#define BUFFER_TYPES                                                        \
    pxr::VtArray<int>, Int2Buffer, Int3Buffer, Int4Buffer, Float1Buffer,           \
        pxr::GfVec2f, pxr::VtVec3fArray, Float4Buffer, Int2, Int3, Int4, Float2, \
        Float3, Float4

#define GEO_SOCKET_TYPES \
    Geometry, MassSpringSocket, SPHFluidSocket, AnimatorSocket, BUFFER_TYPES

#define ALL_SOCKET_TYPES_EXCEPT_ANY                              \
    BASIC_SOCKET_TYPES, STAGE_SOCKET_TYPES, RENDER_SOCKET_TYPES, \
        GEO_SOCKET_TYPES

#define ALL_SOCKET_TYPES_EXCEPT_SPECIAL                              \
    Int, Float, Layer, NumpyArray, SocketGroup, RENDER_SOCKET_TYPES, \
        GEO_SOCKET_TYPES

#define ALL_SOCKET_TYPES ALL_SOCKET_TYPES_EXCEPT_ANY, Any
