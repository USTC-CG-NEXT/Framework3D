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

#include "imgui_nvrhi.h"

#include <imgui.h>
#include <nvrhi/nvrhi.h>
#include <stddef.h>

#include "GUI/api.h"
#include "Logger/Logger.h"
#include "RHI/ShaderFactory/shader.hpp"

const char* vertex_shader_source = R"(
struct Constants {
    float2 invDisplaySize;
};

#ifdef SPIRV

[[vk::push_constant]] ConstantBuffer<Constants> g_Const;

#else

cbuffer g_Const : register(b0)
{
    Constants g_Const;
}

#endif

struct VS_INPUT {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_INPUT {
    float4 out_pos : SV_POSITION;
    float4 out_col : COLOR0;
    float2 out_uv : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.out_pos.xy =
        input.pos.xy * g_Const.invDisplaySize * float2(2.0, -2.0) +
        float2(-1.0, 1.0);
    output.out_pos.zw = float2(0, 1);
    output.out_col = input.col;
    output.out_uv = input.uv;
    return output;
}
)";

const char* pixel_shader_source = R"(
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

Texture2D<float4> texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(PS_INPUT input) : SV_Target
{
    float4 sampledColor = texture0.Sample(sampler0, input.uv);
    float4 out_col = input.col * sampledColor;
    return out_col;
}
)";

USTC_CG_NAMESPACE_OPEN_SCOPE

struct VERTEX_CONSTANT_BUFFER {
    float mvp[4][4];
};

bool ImGui_NVRHI::createFontTexture(nvrhi::ICommandList* commandList)
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    {
        nvrhi::TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = nvrhi::Format::RGBA8_UNORM;
        desc.debugName = "ImGui font texture";

        fontTexture = renderer->createTexture(desc);

        commandList->beginTrackingTextureState(
            fontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);

        if (fontTexture == nullptr)
            return false;

        commandList->writeTexture(fontTexture, 0, 0, pixels, width * 4);

        commandList->setPermanentTextureState(
            fontTexture, nvrhi::ResourceStates::ShaderResource);
        commandList->commitBarriers();

        io.Fonts->TexID = fontTexture;
    }

    {
        const auto desc =
            nvrhi::SamplerDesc()
                .setAllAddressModes(nvrhi::SamplerAddressMode::Wrap)
                .setAllFilters(true);

        fontSampler = renderer->createSampler(desc);

        if (fontSampler == nullptr)
            return false;
    }

    return true;
}

bool ImGui_NVRHI::init(
    nvrhi::DeviceHandle renderer,
    std::shared_ptr<ShaderFactory> shaderFactory)
{
    this->renderer = renderer;

    resource_allocator_.set_device(renderer.Get());

    m_commandList = renderer->createCommandList();

    m_commandList->open();

    std::string error_string;

    ShaderReflectionInfo binding_layout;

    vertexShader = shaderFactory->compile_shader(
        "main",
        nvrhi::ShaderType::Vertex,
        { "imgui_shader/imgui_vertex.slang" },
        binding_layout,
        error_string,
        { { "SPIRV", "1" } },
        std::string(vertex_shader_source));

    pixelShader = shaderFactory->compile_shader(
        "main",
        nvrhi::ShaderType::Pixel,
        "imgui_shader/imgui_pixel.slang",
        binding_layout,
        error_string,
        { { "SPIRV", "1" } },
        std::string(pixel_shader_source));

    if (!vertexShader || !pixelShader) {
        log::error("Failed to create an ImGUI shader");
        return false;
    }

    // create attribute layout object
    nvrhi::VertexAttributeDesc vertexAttribLayout[] = {
        { "POSITION",
          nvrhi::Format::RG32_FLOAT,
          1,
          0,
          offsetof(ImDrawVert, pos),
          sizeof(ImDrawVert),
          false },
        { "TEXCOORD",
          nvrhi::Format::RG32_FLOAT,
          1,
          0,
          offsetof(ImDrawVert, uv),
          sizeof(ImDrawVert),
          false },
        { "COLOR",
          nvrhi::Format::RGBA8_UNORM,
          1,
          0,
          offsetof(ImDrawVert, col),
          sizeof(ImDrawVert),
          false },
    };

    shaderAttribLayout = renderer->createInputLayout(
        vertexAttribLayout,
        sizeof(vertexAttribLayout) / sizeof(vertexAttribLayout[0]),
        vertexShader);

    // add the default font - before creating the font texture
    auto& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    // create font texture
    if (!createFontTexture(m_commandList)) {
        return false;
    }

    // create PSO
    {
        nvrhi::BlendState blendState;
        blendState.targets[0]
            .setBlendEnable(true)
            .setSrcBlend(nvrhi::BlendFactor::SrcAlpha)
            .setDestBlend(nvrhi::BlendFactor::InvSrcAlpha)
            .setSrcBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha)
            .setDestBlendAlpha(nvrhi::BlendFactor::Zero);

        auto rasterState = nvrhi::RasterState()
                               .setFillSolid()
                               .setCullNone()
                               .setScissorEnable(true)
                               .setDepthClipEnable(true);

        auto depthStencilState =
            nvrhi::DepthStencilState()
                .disableDepthTest()
                .enableDepthWrite()
                .disableStencil()
                .setDepthFunc(nvrhi::ComparisonFunc::Always);

        nvrhi::RenderState renderState;
        renderState.blendState = blendState;
        renderState.depthStencilState = depthStencilState;
        renderState.rasterState = rasterState;

        nvrhi::BindingLayoutDesc layoutDesc;
        layoutDesc.visibility = nvrhi::ShaderType::All;
        layoutDesc.bindings = { nvrhi::BindingLayoutItem::PushConstants(
                                    0, sizeof(float) * 2),
                                nvrhi::BindingLayoutItem::Texture_SRV(0),
                                nvrhi::BindingLayoutItem::Sampler(0) };
        bindingLayout = renderer->createBindingLayout(layoutDesc);

        basePSODesc.primType = nvrhi::PrimitiveType::TriangleList;
        basePSODesc.inputLayout = shaderAttribLayout;
        basePSODesc.VS = vertexShader;
        basePSODesc.PS = pixelShader;
        basePSODesc.renderState = renderState;
        basePSODesc.bindingLayouts = { bindingLayout };
    }

    m_commandList->close();
    renderer->executeCommandList(m_commandList);

    return true;
}

bool ImGui_NVRHI::reallocateBuffer(
    nvrhi::BufferHandle& buffer,
    size_t requiredSize,
    size_t reallocateSize,
    const bool indexBuffer)
{
    if (buffer == nullptr ||
        size_t(buffer->getDesc().byteSize) < requiredSize) {
        nvrhi::BufferDesc desc;
        desc.byteSize = uint32_t(reallocateSize);
        desc.structStride = 0;
        desc.debugName =
            indexBuffer ? "ImGui index buffer" : "ImGui vertex buffer";
        desc.canHaveUAVs = false;
        desc.isVertexBuffer = !indexBuffer;
        desc.isIndexBuffer = indexBuffer;
        desc.isDrawIndirectArgs = false;
        desc.isVolatile = false;
        desc.initialState = indexBuffer ? nvrhi::ResourceStates::IndexBuffer
                                        : nvrhi::ResourceStates::VertexBuffer;
        desc.keepInitialState = true;

        buffer = renderer->createBuffer(desc);

        if (!buffer) {
            return false;
        }
    }

    return true;
}

bool ImGui_NVRHI::beginFrame(float elapsedTimeSeconds)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = elapsedTimeSeconds;
    io.MouseDrawCursor = false;

    ImGui::NewFrame();

    return true;
}

nvrhi::IGraphicsPipeline* ImGui_NVRHI::getPSO(nvrhi::IFramebuffer* fb)
{
    if (pso)
        return pso;

    pso = renderer->createGraphicsPipeline(basePSODesc, fb);
    assert(pso);

    return pso;
}

nvrhi::BindingSetHandle ImGui_NVRHI::getBindingSet(nvrhi::ITexture* texture)
{
    nvrhi::BindingSetDesc desc;

    desc.bindings = { nvrhi::BindingSetItem::PushConstants(
                          0, sizeof(float) * 2),
                      nvrhi::BindingSetItem::Texture_SRV(0, texture),
                      nvrhi::BindingSetItem::Sampler(0, fontSampler) };

    binding = resource_allocator_.create(desc, bindingLayout);
    assert(binding);

    return binding;
}

bool ImGui_NVRHI::updateGeometry(nvrhi::ICommandList* commandList)
{
    ImDrawData* drawData = ImGui::GetDrawData();

    // create/resize vertex and index buffers if needed
    if (!reallocateBuffer(
            vertexBuffer,
            drawData->TotalVtxCount * sizeof(ImDrawVert),
            (drawData->TotalVtxCount + 5000) * sizeof(ImDrawVert),
            false)) {
        return false;
    }

    if (!reallocateBuffer(
            indexBuffer,
            drawData->TotalIdxCount * sizeof(ImDrawIdx),
            (drawData->TotalIdxCount + 5000) * sizeof(ImDrawIdx),
            true)) {
        return false;
    }

    vtxBuffer.resize(vertexBuffer->getDesc().byteSize / sizeof(ImDrawVert));
    idxBuffer.resize(indexBuffer->getDesc().byteSize / sizeof(ImDrawIdx));

    // copy and convert all vertices into a single contiguous buffer
    ImDrawVert* vtxDst = &vtxBuffer[0];
    ImDrawIdx* idxDst = &idxBuffer[0];

    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        memcpy(
            vtxDst,
            cmdList->VtxBuffer.Data,
            cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(
            idxDst,
            cmdList->IdxBuffer.Data,
            cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        vtxDst += cmdList->VtxBuffer.Size;
        idxDst += cmdList->IdxBuffer.Size;
    }

    commandList->writeBuffer(
        vertexBuffer, &vtxBuffer[0], vertexBuffer->getDesc().byteSize);
    commandList->writeBuffer(
        indexBuffer, &idxBuffer[0], indexBuffer->getDesc().byteSize);

    return true;
}

bool ImGui_NVRHI::render(nvrhi::IFramebuffer* framebuffer)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    const auto& io = ImGui::GetIO();

    m_commandList->open();
    m_commandList->beginMarker("ImGUI");
    m_commandList->clearTextureFloat(
        framebuffer->getDesc().colorAttachments[0].texture,
        {},
        nvrhi::Color{ 0 });

    if (!updateGeometry(m_commandList)) {
        return false;
    }

    // handle DPI scaling
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    float invDisplaySize[2] = { 1.f / io.DisplaySize.x,
                                1.f / io.DisplaySize.y };

    // set up graphics state
    nvrhi::GraphicsState drawState;

    drawState.framebuffer = framebuffer;
    assert(drawState.framebuffer);

    drawState.pipeline = getPSO(drawState.framebuffer);

    drawState.viewport.viewports.push_back(nvrhi::Viewport(
        io.DisplaySize.x * io.DisplayFramebufferScale.x,
        io.DisplaySize.y * io.DisplayFramebufferScale.y));
    drawState.viewport.scissorRects.resize(1);  // updated below

    nvrhi::VertexBufferBinding vbufBinding;
    vbufBinding.buffer = vertexBuffer;
    vbufBinding.slot = 0;
    vbufBinding.offset = 0;
    drawState.vertexBuffers.push_back(vbufBinding);

    drawState.indexBuffer.buffer = indexBuffer;
    drawState.indexBuffer.format =
        (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT
                                : nvrhi::Format::R32_UINT);
    drawState.indexBuffer.offset = 0;

    // render command lists
    int vtxOffset = 0;
    int idxOffset = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
            const ImDrawCmd* pCmd = &cmdList->CmdBuffer[i];

            if (pCmd->UserCallback) {
                pCmd->UserCallback(cmdList, pCmd);
            }
            else {
                auto bindingSet =
                    getBindingSet((nvrhi::ITexture*)pCmd->TextureId);
                drawState.bindings = { bindingSet };
                assert(drawState.bindings[0]);

                drawState.viewport.scissorRects[0] = nvrhi::Rect(
                    int(pCmd->ClipRect.x),
                    int(pCmd->ClipRect.z),
                    int(pCmd->ClipRect.y),
                    int(pCmd->ClipRect.w));

                nvrhi::DrawArguments drawArguments;
                drawArguments.vertexCount = pCmd->ElemCount;
                drawArguments.startIndexLocation = idxOffset;
                drawArguments.startVertexLocation = vtxOffset;

                m_commandList->setGraphicsState(drawState);
                m_commandList->setPushConstants(
                    invDisplaySize, sizeof(invDisplaySize));
                m_commandList->drawIndexed(drawArguments);

                resource_allocator_.destroy(bindingSet);
            }

            idxOffset += pCmd->ElemCount;
        }

        vtxOffset += cmdList->VtxBuffer.Size;
    }

    m_commandList->endMarker();
    m_commandList->close();
    renderer->executeCommandList(m_commandList);

    return true;
}

void ImGui_NVRHI::backbufferResizing()
{
    pso = nullptr;
}
USTC_CG_NAMESPACE_CLOSE_SCOPE