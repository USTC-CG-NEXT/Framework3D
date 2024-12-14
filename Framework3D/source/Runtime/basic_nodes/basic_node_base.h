#pragma once
#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/vt/array.h"

using float1Buffer = pxr::VtArray<float>;
using float2Buffer = pxr::VtArray<pxr::GfVec2f>;
using float3Buffer = pxr::VtArray<pxr::GfVec3f>;
using float4Buffer = pxr::VtArray<pxr::GfVec4f>;

using int1Buffer = pxr::VtArray<int>;
using int2Buffer = pxr::VtArray<pxr::GfVec2i>;
using int3Buffer = pxr::VtArray<pxr::GfVec3i>;
using int4Buffer = pxr::VtArray<pxr::GfVec4i>;