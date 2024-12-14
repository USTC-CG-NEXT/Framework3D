/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once

/**
 * Compilers.
 */
#define FALCOR_COMPILER_MSVC  1
#define FALCOR_COMPILER_CLANG 2
#define FALCOR_COMPILER_GCC   3

/**
 * Determine the compiler in use.
 * http://sourceforge.net/p/predef/wiki/Compilers/
 */
#ifndef FALCOR_COMPILER
#if defined(_MSC_VER)
#define FALCOR_COMPILER FALCOR_COMPILER_MSVC
#elif defined(__clang__)
#define FALCOR_COMPILER FALCOR_COMPILER_CLANG
#elif defined(__GNUC__)
#define FALCOR_COMPILER FALCOR_COMPILER_GCC
#else
#error "Unsupported compiler"
#endif
#endif  // FALCOR_COMPILER

#define FALCOR_MSVC  (FALCOR_COMPILER == FALCOR_COMPILER_MSVC)
#define FALCOR_CLANG (FALCOR_COMPILER == FALCOR_COMPILER_CLANG)
#define FALCOR_GCC   (FALCOR_COMPILER == FALCOR_COMPILER_GCC)

/**
 * Platforms.
 */
#define FALCOR_PLATFORM_WINDOWS 1
#define FALCOR_PLATFORM_LINUX   2

/**
 * Determine the target platform in use.
 * http://sourceforge.net/p/predef/wiki/OperatingSystems/
 */
#ifndef FALCOR_PLATFORM
#if defined(_WIN64)
#define FALCOR_PLATFORM FALCOR_PLATFORM_WINDOWS
#elif defined(__linux__)
#define FALCOR_PLATFORM FALCOR_PLATFORM_LINUX
#else
#error "Unsupported target platform"
#endif
#endif  // FALCOR_PLATFORM

#define FALCOR_WINDOWS (FALCOR_PLATFORM == FALCOR_PLATFORM_WINDOWS)
#define FALCOR_LINUX   (FALCOR_PLATFORM == FALCOR_PLATFORM_LINUX)

/**
 * D3D12 Agility SDK.
 */
#if FALCOR_HAS_D3D12_AGILITY_SDK
#define FALCOR_D3D12_AGILITY_SDK_VERSION 4
#define FALCOR_D3D12_AGILITY_SDK_PATH    ".\\D3D12\\"
// To enable the D3D12 Agility SDK, this macro needs to be added to the main
// source file of the executable.
#define FALCOR_EXPORT_D3D12_AGILITY_SDK                               \
    extern "C" {                                                      \
    HD_USTC_CG_API_EXPORT extern const unsigned int D3D12SDKVersion = \
        FALCOR_D3D12_AGILITY_SDK_VERSION;                             \
    }                                                                 \
    extern "C" {                                                      \
    HD_USTC_CG_API_EXPORT extern const char* D3D12SDKPath =           \
        FALCOR_D3D12_AGILITY_SDK_PATH;                                \
    }
#else
#define FALCOR_EXPORT_D3D12_AGILITY_SDK
#endif

/**
 * Define for checking if NVAPI is available.
 */
#define FALCOR_NVAPI_AVAILABLE (1 && FALCOR_HAS_NVAPI)

/**
 * Force inline.
 */
#if FALCOR_MSVC
#define FALCOR_FORCEINLINE __forceinline
#elif FALCOR_CLANG | FALCOR_GCC
#define FALCOR_FORCEINLINE __attribute__((always_inline))
#endif

/**
 * Preprocessor stringification.
 */
#define FALCOR_STRINGIZE(a)          #a
#define FALCOR_CONCAT_STRINGS_(a, b) a##b
#define FALCOR_CONCAT_STRINGS(a, b)  FALCOR_CONCAT_STRINGS_(a, b)

/**
 * Implement logical operators on a class enum for making it usable as a flags
 * enum.
 */
// clang-format off
#define FALCOR_ENUM_CLASS_OPERATORS(e_) \
    inline e_ operator& (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b)); } \
    inline e_ operator| (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b)); } \
    inline e_& operator|= (e_& a, e_ b) { a = a | b; return a; }; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a)); } \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != static_cast<e_>(0); } \
    inline void flip_bit(e_& val, e_ flag) { val = is_set(val, flag) ? (val & (~flag)) : (val | flag); }
// clang-format on

#include <nvrhi/nvrhi.h>

using Texture = nvrhi::ITexture;
using Buffer = nvrhi::IBuffer;
using Resource = nvrhi::IResource;
using Fbo = nvrhi::IFramebuffer;
using Sampler = nvrhi::ISampler;

struct ShaderOffset {
    int uniformOffset = 0;  // TODO: Change to Offset?
    int bindingRangeIndex = 0;
    int bindingArrayIndex = 0;
};

using RtAccelerationStructure = nvrhi::rt::IAccelStruct;
using nvrhi::IDevice;
using nvrhi::IResource;
using nvrhi::ResourceStates;
using nvrhi::ShaderType;
using Vao = nvrhi::VertexBufferBinding;
using nvrhi::BlendState;
using nvrhi::DepthStencilState;
using RasterizerState = nvrhi::RasterState;

using VertexLayout = nvrhi::VertexAttributeDesc;

using nvrhi::ResourceType;

using TextureReductionMode = nvrhi::SamplerReductionType;

using ClearValue = nvrhi::Color;
using nvrhi::SamplerAddressMode;
using nvrhi::SamplerDesc;
using nvrhi::Viewport;

using Resource = nvrhi::IResource;

using UnorderedAccessView = nvrhi::BindingSetItem;

using GpuMemoryHeap = nvrhi::IHeap;
using QueryHeap = nvrhi::ITimerQuery;

using Fence = nvrhi::IEventQuery;
using DepthStencilView = nvrhi::BindingSetItem;
