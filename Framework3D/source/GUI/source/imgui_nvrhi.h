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

/*
License for Dear ImGui

Copyright (c) 2014-2019 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <imgui.h>
#include <nvrhi/nvrhi.h>

#include <memory>
#include <vector>

#define RESOURCE_ALLOCATOR_STATIC_ONLY
#include "RHI/ResourceManager/resource_allocator.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class ShaderFactory;

struct ImGui_NVRHI {
    nvrhi::DeviceHandle renderer;
    nvrhi::CommandListHandle m_commandList;

    nvrhi::ShaderHandle vertexShader;
    nvrhi::ShaderHandle pixelShader;
    nvrhi::InputLayoutHandle shaderAttribLayout;

    nvrhi::TextureHandle fontTexture;
    nvrhi::SamplerHandle fontSampler;

    nvrhi::BufferHandle vertexBuffer;
    nvrhi::BufferHandle indexBuffer;

    nvrhi::BindingLayoutHandle bindingLayout;
    nvrhi::GraphicsPipelineDesc basePSODesc;

    nvrhi::GraphicsPipelineHandle pso;

    std::vector<ImDrawVert> vtxBuffer;
    std::vector<ImDrawIdx> idxBuffer;
    nvrhi::BindingSetHandle binding;

    bool init(
        nvrhi::DeviceHandle renderer,
        std::shared_ptr<ShaderFactory> shaderFactory);
    bool beginFrame(float elapsedTimeSeconds);
    bool render(nvrhi::IFramebuffer* framebuffer);
    void backbufferResizing();

   private:
    bool reallocateBuffer(
        nvrhi::BufferHandle& buffer,
        size_t requiredSize,
        size_t reallocateSize,
        bool isIndexBuffer);

    bool createFontTexture(nvrhi::ICommandList* commandList);

    nvrhi::IGraphicsPipeline* getPSO(nvrhi::IFramebuffer* fb);
    nvrhi::BindingSetHandle getBindingSet(nvrhi::ITexture* texture);
    bool updateGeometry(nvrhi::ICommandList* commandList);

    ResourceAllocator resource_allocator_;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE