//
// Copyright 2018 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "renderBuffer.h"

#include <iostream>

#include "RHI/Hgi/format_conversion.hpp"
#include "RHI/rhi.hpp"
#include "pxr/base/gf/half.h"
#include "renderParam.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

Hd_USTC_CG_RenderBuffer::Hd_USTC_CG_RenderBuffer(SdfPath const &id)
    : HdRenderBuffer(id),
      _width(0),
      _height(0),
      _format(HdFormatInvalid),
      _multiSampled(false),
      _buffer(),
      _mappers(0),
      _converged(false),
      name(id.GetString())
{
}

Hd_USTC_CG_RenderBuffer::~Hd_USTC_CG_RenderBuffer()
{
    staging = nullptr;
    m_CommandList = nullptr;
}

/*virtual*/
void Hd_USTC_CG_RenderBuffer::Sync(
    HdSceneDelegate *sceneDelegate,
    HdRenderParam *renderParam,
    HdDirtyBits *dirtyBits)
{
    auto ustc_renderParam = static_cast<Hd_USTC_CG_RenderParam *>(renderParam);
    nvrhi_device = RHI::get_device();
    HdRenderBuffer::Sync(sceneDelegate, renderParam, dirtyBits);
}

/*virtual*/
void Hd_USTC_CG_RenderBuffer::Finalize(HdRenderParam *renderParam)
{
    HdRenderBuffer::Finalize(renderParam);
}

/*virtual*/
void Hd_USTC_CG_RenderBuffer::_Deallocate()
{
    // If the buffer is mapped while we're doing this, there's not a great
    // recovery path...
    TF_VERIFY(!IsMapped());

    staging = nullptr;

    _width = 0;
    _height = 0;
    _format = HdFormatInvalid;
    _multiSampled = false;
    _buffer.resize(0);

    _mappers.store(0);
    _converged.store(false);
}

void Hd_USTC_CG_RenderBuffer::Resolve()
{
}

bool Hd_USTC_CG_RenderBuffer::Allocate(
    GfVec3i const &dimensions,
    HdFormat format,
    bool multiSampled)
{
    _Deallocate();

    _width = dimensions[0];
    _height = dimensions[1];
    _format = format;

    nvrhi::TextureDesc d;
    d.debugName = name + "_staging";
    d.width = _width;
    d.height = _height;
    d.format = RHI::ConvertToNvrhiFormat(format);  // TODO
    d.initialState = nvrhi::ResourceStates::CopyDest;
    staging = nvrhi_device->createStagingTexture(d, nvrhi::CpuAccessMode::Read);

    _buffer.resize(
        _width * _height *
        RHI::calculate_bytes_per_pixel(RHI::ConvertToNvrhiFormat(format)));

    _multiSampled = multiSampled;

    return true;
}

VtValue Hd_USTC_CG_RenderBuffer::GetResource(bool multiSampled) const
{
    return VtValue(pxr::HgiTextureHandle{});
}

void Hd_USTC_CG_RenderBuffer::Clear()
{
}

void Hd_USTC_CG_RenderBuffer::Present(nvrhi::TextureHandle handle)
{
#ifndef USTC_CG_DIRECT_VK_DISPLAY
    if (!m_CommandList) {
        m_CommandList = nvrhi_device->createCommandList();
    }
    m_CommandList->open();

    m_CommandList->copyTexture(staging, {}, handle, {});
    m_CommandList->close();

    nvrhi_device->executeCommandList(m_CommandList.Get());

    size_t pitch;
    auto mapped = nvrhi_device->mapStagingTexture(
        staging, {}, nvrhi::CpuAccessMode::Read, &pitch);

    for (int i = 0; i < handle->getDesc().height; ++i) {
        memcpy(
            _buffer.data() + i * _width * HdDataSizeOfFormat(_format),
            (uint8_t *)mapped + i * pitch,
            _width * HdDataSizeOfFormat(_format));
    }

    nvrhi_device->unmapStagingTexture(staging);
#endif
}
void *Hd_USTC_CG_RenderBuffer::Map()
{
    nvrhi_device->waitForIdle();
    _mappers++;
    return _buffer.data();
}

void Hd_USTC_CG_RenderBuffer::Unmap()
{
    _mappers--;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
