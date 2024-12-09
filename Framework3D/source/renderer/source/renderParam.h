//
// Copyright 2017 Pixar
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
#pragma once

#include "DescriptorTableManager.h"
#include "RHI/rhi.hpp"
#include "api.h"
#include "nodes/system/node_system.hpp"
#include "pxr/imaging/hd/renderDelegate.h"
#include "pxr/imaging/hd/renderThread.h"
#include "pxr/pxr.h"
#include "renderTLAS.h"

namespace USTC_CG {
class LensSystem;
}

namespace USTC_CG {
struct RenderGlobalPayload;
}

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

struct BindlessData {
    BindlessData()
    {
        auto device = RHI::get_device();
        nvrhi::BindlessLayoutDesc desc;
        desc.visibility = nvrhi::ShaderType::All;
        desc.maxCapacity = 8 * 1024;
        bindlessLayout = device->createBindlessLayout(desc);
        descriptorTableManager =
            std::make_unique<DescriptorTableManager>(device, bindlessLayout);
    }

    std::unique_ptr<DescriptorTableManager> descriptorTableManager;

   private:
    nvrhi::BindingLayoutHandle bindlessLayout;
};

///
/// \class Hd_USTC_CG_RenderParam
///
/// The render delegate can create an object of type HdRenderParam, to pass
/// to each prim during Sync(). Hd_USTC_CG uses this class to pass top-level
/// embree state around.
///
class Hd_USTC_CG_RenderParam final : public HdRenderParam {
   public:
    Hd_USTC_CG_RenderParam(
        HdRenderThread *renderThread,
        std::atomic<int> *sceneVersion,
        NodeSystem *node_system)
        : _renderThread(renderThread),
          _sceneVersion(sceneVersion),
          node_system(node_system)
    {
        TLAS = std::make_unique<Hd_USTC_CG_RenderTLAS>();
    }
    ~Hd_USTC_CG_RenderParam()
    {
    }

    DescriptorTableManager *get_descriptor_table() const;

    HdRenderThread *_renderThread = nullptr;

    NodeSystem *node_system;
    std::unique_ptr<Hd_USTC_CG_RenderTLAS> TLAS;

    nvrhi::TextureHandle presented_texture;
    LensSystem *lens_system;

   private:
    /// A handle to the global render thread.
    /// A version counter for edits to _scene.
    std::atomic<int> *_sceneVersion;
    BindlessData bindlessData;
};

inline DescriptorTableManager *Hd_USTC_CG_RenderParam::get_descriptor_table()
    const
{
    return bindlessData.descriptorTableManager.get();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
