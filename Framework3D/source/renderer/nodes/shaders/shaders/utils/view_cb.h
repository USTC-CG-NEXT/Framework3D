/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef VIEW_CB_H
#define VIEW_CB_H

#include "cpp_shader_macro.h"

struct PlanarViewConstants {
#ifdef __cplusplus
    USING_PXR_MATH_TYPES
    friend bool operator==(
        const PlanarViewConstants& lhs,
        const PlanarViewConstants& rhs)
    {
        return lhs.matWorldToClip == rhs.matWorldToClip &&
               lhs.resolution == rhs.resolution &&
               lhs.viewportOrigin == rhs.viewportOrigin &&
               lhs.viewportSize == rhs.viewportSize &&
               lhs.pixelOffset == rhs.pixelOffset;
    }

    friend bool operator!=(
        const PlanarViewConstants& lhs,
        const PlanarViewConstants& rhs)
    {
        return !(lhs == rhs);
    }
#endif

    float4x4 matWorldToView;
    float4x4 matViewToClip;
    float4x4 matWorldToClip;
    float4x4 matClipToView;
    float4x4 matViewToWorld;
    float4x4 matClipToWorld;

    float2 viewportOrigin;
    float2 viewportSize;

    float2 viewportSizeInv;
    float2 pixelOffset;

    float4 cameraDirectionOrPosition;

    int2 resolution;
    int2 padding;
};

#endif  // VIEW_CB_H