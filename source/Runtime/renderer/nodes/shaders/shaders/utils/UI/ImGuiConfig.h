/***************************************************************************
 # Copyright (c) 2015-24, NVIDIA CORPORATION. All rights reserved.
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

// ImGui configuration file. See imconfig.h for more details.
// We use this to provide implicit conversion between ImGui's vector types
// and Falcor's vector types.

#include "utils/Math/VectorTypes.h"

// clang-format off
#define IM_VEC2_CLASS_EXTRA                                                             \
    constexpr ImVec2(const ::USTC_CG::float2& f) : x(f.x), y(f.y) {}                     \
    operator ::USTC_CG::float2() const { return ::USTC_CG::float2(x, y); }

#define IM_VEC3_CLASS_EXTRA                                                             \
    constexpr ImVec3(const ::USTC_CG::float3& f) : x(f.x), y(f.y), z(f.z) {}             \
    operator ::USTC_CG::float3() const { return ::USTC_CG::float3(x, y, z); }

#define IM_VEC4_CLASS_EXTRA                                                             \
    constexpr ImVec4(const ::USTC_CG::float4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}     \
    operator ::USTC_CG::float4() const { return ::USTC_CG::float4(x, y, z, w); }
// clang-format on
