#pragma once

#include "USTC_CG.h"
#define BASIC_SOCKET_TYPES Int, String, Float

#define STAGE_SOCKET_TYPES Layer, PyObj, NumpyArray, TorchTensor, SocketGroup

#define RENDER_SOCKET_TYPES \
    Lights, Camera, Texture, Meshes, Materials, AccelStruct, Buffer

#define BUFFER_TYPES                                                        \
    Int1Buffer, Int2Buffer, Int3Buffer, Int4Buffer, Float1Buffer,           \
        Float2Buffer, Float3Buffer, Float4Buffer, Int2, Int3, Int4, Float2, \
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
