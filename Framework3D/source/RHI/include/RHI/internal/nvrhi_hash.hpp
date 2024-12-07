#pragma once
#include "RHI/api.h"
#include "nvrhi/nvrhi.h"
#include "nvrhi_patch.hpp"
#include "resources.hpp"

namespace std {

template<>
struct hash<USTC_CG::ProgramDesc> {
    size_t operator()(const USTC_CG::ProgramDesc& s) const noexcept
    {
        size_t seed = 0;
        nvrhi::hash_combine(seed, s.path);
        nvrhi::hash_combine(seed, s.entry_name);
        nvrhi::hash_combine(seed, s.lastWriteTime.time_since_epoch().count());
        nvrhi::hash_combine(seed, s.shaderType);
        nvrhi::hash_combine(seed, s.nvapi_support);
        return seed;
    }
};

template<>
struct hash<nvrhi::BindingLayoutItem> {
    std::size_t operator()(nvrhi::BindingLayoutItem const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.slot);
        nvrhi::hash_combine(seed, s.type);
        nvrhi::hash_combine(seed, s.size);
        return seed;
    }
};

template<>
struct hash<nvrhi::VariableRateShadingState> {
    std::size_t operator()(
        nvrhi::VariableRateShadingState const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.enabled);
        nvrhi::hash_combine(seed, s.shadingRate);
        nvrhi::hash_combine(seed, s.pipelinePrimitiveCombiner);
        nvrhi::hash_combine(seed, s.imageCombiner);
        return seed;
    }
};

template<>
struct hash<nvrhi::SinglePassStereoState> {
    std::size_t operator()(nvrhi::SinglePassStereoState const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.enabled);
        nvrhi::hash_combine(seed, s.independentViewportMask);
        nvrhi::hash_combine(seed, s.renderTargetIndexOffset);

        return seed;
    }
};

template<>
struct hash<nvrhi::Color> {
    std::size_t operator()(nvrhi::Color const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.r);
        nvrhi::hash_combine(seed, s.g);
        nvrhi::hash_combine(seed, s.b);
        nvrhi::hash_combine(seed, s.a);
        return seed;
    }
};

template<>
struct hash<nvrhi::CommandListParameters> {
    std::size_t operator()(nvrhi::CommandListParameters const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.enableImmediateExecution);
        nvrhi::hash_combine(seed, s.uploadChunkSize);
        nvrhi::hash_combine(seed, s.scratchChunkSize);
        nvrhi::hash_combine(seed, s.scratchMaxMemory);
        nvrhi::hash_combine(seed, s.queueType);
        return seed;
    }
};

template<>
struct hash<nvrhi::ShaderDesc> {
    std::size_t operator()(nvrhi::ShaderDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.shaderType);
        nvrhi::hash_combine(seed, s.debugName);
        nvrhi::hash_combine(seed, s.entryName);
        nvrhi::hash_combine(seed, s.hlslExtensionsUAV);
        nvrhi::hash_combine(seed, s.useSpecificShaderExt);
        nvrhi::hash_combine(seed, s.numCustomSemantics);
        nvrhi::hash_combine(seed, s.pCustomSemantics);
        nvrhi::hash_combine(seed, s.fastGSFlags);
        nvrhi::hash_combine(seed, s.pCoordinateSwizzling);
        return seed;
    }
};

template<>
struct hash<nvrhi::TextureDesc> {
    std::size_t operator()(nvrhi::TextureDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.width);
        nvrhi::hash_combine(seed, s.height);
        nvrhi::hash_combine(seed, s.depth);
        nvrhi::hash_combine(seed, s.arraySize);
        nvrhi::hash_combine(seed, s.mipLevels);
        nvrhi::hash_combine(seed, s.sampleCount);
        nvrhi::hash_combine(seed, s.sampleQuality);
        nvrhi::hash_combine(seed, s.format);
        nvrhi::hash_combine(seed, s.dimension);
        nvrhi::hash_combine(seed, s.isShaderResource);
        nvrhi::hash_combine(seed, s.isRenderTarget);
        nvrhi::hash_combine(seed, s.isUAV);
        nvrhi::hash_combine(seed, s.isTypeless);
        nvrhi::hash_combine(seed, s.isShadingRateSurface);
        nvrhi::hash_combine(seed, s.sharedResourceFlags);
        nvrhi::hash_combine(seed, s.isVirtual);
        nvrhi::hash_combine(seed, s.clearValue);
        nvrhi::hash_combine(seed, s.useClearValue);
        nvrhi::hash_combine(seed, s.initialState);
        nvrhi::hash_combine(seed, s.keepInitialState);
        return seed;
    }
};

template<>
struct hash<nvrhi::BufferDesc> {
    std::size_t operator()(nvrhi::BufferDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.byteSize);
        nvrhi::hash_combine(seed, s.structStride);
        nvrhi::hash_combine(seed, s.maxVersions);
        nvrhi::hash_combine(seed, s.debugName);
        nvrhi::hash_combine(seed, s.format);
        nvrhi::hash_combine(seed, s.canHaveUAVs);
        nvrhi::hash_combine(seed, s.canHaveTypedViews);
        nvrhi::hash_combine(seed, s.canHaveRawViews);
        nvrhi::hash_combine(seed, s.isVertexBuffer);
        nvrhi::hash_combine(seed, s.isIndexBuffer);
        nvrhi::hash_combine(seed, s.isConstantBuffer);
        nvrhi::hash_combine(seed, s.isDrawIndirectArgs);
        nvrhi::hash_combine(seed, s.isAccelStructBuildInput);
        nvrhi::hash_combine(seed, s.isAccelStructStorage);
        nvrhi::hash_combine(seed, s.isShaderBindingTable);
        nvrhi::hash_combine(seed, s.isVolatile);
        nvrhi::hash_combine(seed, s.isVirtual);
        nvrhi::hash_combine(seed, s.initialState);
        nvrhi::hash_combine(seed, s.keepInitialState);
        nvrhi::hash_combine(seed, s.cpuAccess);
        nvrhi::hash_combine(seed, s.sharedResourceFlags);
        return seed;
    }
};

template<typename T, uint32_t _max_elements>
struct hash<nvrhi::static_vector<T, _max_elements>> {
    std::size_t operator()(
        nvrhi::static_vector<T, _max_elements> const& s) const noexcept
    {
        std::size_t seed = 0;
        for (const auto& elem : s) {
            nvrhi::hash_combine(seed, elem);
        }
        return seed;
    }
};

template<>
struct hash<nvrhi::VulkanBindingOffsets> {
    std::size_t operator()(nvrhi::VulkanBindingOffsets const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.shaderResource);
        nvrhi::hash_combine(seed, s.sampler);
        nvrhi::hash_combine(seed, s.constantBuffer);
        nvrhi::hash_combine(seed, s.unorderedAccess);
        return seed;
    }
};

template<>
struct hash<nvrhi::BindingLayoutDesc> {
    std::size_t operator()(nvrhi::BindingLayoutDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.visibility);
        nvrhi::hash_combine(seed, s.registerSpace);
        nvrhi::hash_combine(seed, s.registerSpaceIsDescriptorSet);
        nvrhi::hash_combine(seed, s.bindings);
        nvrhi::hash_combine(seed, s.bindingOffsets);
        return seed;
    }
};

template<>
struct hash<nvrhi::StagingTextureDesc> {
    std::size_t operator()(nvrhi::StagingTextureDesc const& s) const noexcept
    {
        return hash<nvrhi::TextureDesc>()(
            static_cast<const nvrhi::TextureDesc&>(s));
    }
};

template<>
struct hash<nvrhi::ComputePipelineDesc> {
    std::size_t operator()(nvrhi::ComputePipelineDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.CS);
        nvrhi::hash_combine(seed, s.bindingLayouts);
        return seed;
    }
};

template<>
struct hash<nvrhi::FramebufferAttachment> {
    std::size_t operator()(nvrhi::FramebufferAttachment const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.texture);
        nvrhi::hash_combine(seed, s.subresources);
        nvrhi::hash_combine(seed, s.format);
        nvrhi::hash_combine(seed, s.isReadOnly);
        return seed;
    }
};

template<>
struct hash<nvrhi::FramebufferDesc> {
    std::size_t operator()(nvrhi::FramebufferDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.colorAttachments);
        nvrhi::hash_combine(seed, s.depthAttachment);
        nvrhi::hash_combine(seed, s.shadingRateAttachment);
        return seed;
    }
};

template<>
struct hash<nvrhi::SamplerDesc> {
    std::size_t operator()(nvrhi::SamplerDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.borderColor);
        nvrhi::hash_combine(seed, s.maxAnisotropy);
        nvrhi::hash_combine(seed, s.mipBias);
        nvrhi::hash_combine(seed, s.minFilter);
        nvrhi::hash_combine(seed, s.magFilter);
        nvrhi::hash_combine(seed, s.mipFilter);
        nvrhi::hash_combine(seed, s.addressU);
        nvrhi::hash_combine(seed, s.addressV);
        nvrhi::hash_combine(seed, s.addressW);
        nvrhi::hash_combine(seed, s.reductionType);
        return seed;
    }
};

template<>
struct hash<nvrhi::DepthStencilState::StencilOpDesc> {
    std::size_t operator()(
        nvrhi::DepthStencilState::StencilOpDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.failOp);
        nvrhi::hash_combine(seed, s.depthFailOp);
        nvrhi::hash_combine(seed, s.passOp);
        nvrhi::hash_combine(seed, s.stencilFunc);
        return seed;
    }
};

template<>
struct hash<nvrhi::VertexAttributeDesc> {
    std::size_t operator()(nvrhi::VertexAttributeDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.name);
        nvrhi::hash_combine(seed, s.format);
        nvrhi::hash_combine(seed, s.arraySize);
        nvrhi::hash_combine(seed, s.bufferIndex);
        nvrhi::hash_combine(seed, s.offset);
        nvrhi::hash_combine(seed, s.elementStride);
        nvrhi::hash_combine(seed, s.isInstanced);
        return seed;
    }
};

template<>
struct hash<nvrhi::DepthStencilState> {
    std::size_t operator()(nvrhi::DepthStencilState const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.depthTestEnable);
        nvrhi::hash_combine(seed, s.depthWriteEnable);
        nvrhi::hash_combine(seed, s.depthFunc);
        nvrhi::hash_combine(seed, s.stencilEnable);
        nvrhi::hash_combine(seed, s.stencilReadMask);
        nvrhi::hash_combine(seed, s.stencilWriteMask);
        nvrhi::hash_combine(seed, s.stencilRefValue);
        nvrhi::hash_combine(seed, s.dynamicStencilRef);
        nvrhi::hash_combine(seed, s.frontFaceStencil);
        nvrhi::hash_combine(seed, s.backFaceStencil);
        return seed;
    }
};

template<>
struct hash<nvrhi::RasterState> {
    std::size_t operator()(nvrhi::RasterState const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.fillMode);
        nvrhi::hash_combine(seed, s.cullMode);
        nvrhi::hash_combine(seed, s.frontCounterClockwise);
        nvrhi::hash_combine(seed, s.depthClipEnable);
        nvrhi::hash_combine(seed, s.scissorEnable);
        nvrhi::hash_combine(seed, s.multisampleEnable);
        nvrhi::hash_combine(seed, s.antialiasedLineEnable);
        nvrhi::hash_combine(seed, s.depthBias);
        nvrhi::hash_combine(seed, s.depthBiasClamp);
        nvrhi::hash_combine(seed, s.slopeScaledDepthBias);
        nvrhi::hash_combine(seed, s.forcedSampleCount);
        nvrhi::hash_combine(seed, s.programmableSamplePositionsEnable);
        nvrhi::hash_combine(seed, s.conservativeRasterEnable);
        nvrhi::hash_combine(seed, s.quadFillEnable);
        return seed;
    }
};

template<>
struct hash<nvrhi::RenderState> {
    std::size_t operator()(nvrhi::RenderState const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.blendState);
        nvrhi::hash_combine(seed, s.depthStencilState);
        nvrhi::hash_combine(seed, s.rasterState);
        nvrhi::hash_combine(seed, s.singlePassStereo);
        return seed;
    }
};

template<>
struct hash<nvrhi::GraphicsPipelineDesc> {
    std::size_t operator()(nvrhi::GraphicsPipelineDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        if (s.inputLayout != nullptr) {
            for (int i = 0; i < s.inputLayout->getNumAttributes(); ++i) {
                auto attrs = s.inputLayout->getAttributeDesc(i);
                nvrhi::hash_combine(seed, *attrs);
            }
        }
        nvrhi::hash_combine(seed, s.primType);
        nvrhi::hash_combine(seed, s.patchControlPoints);
        nvrhi::hash_combine(seed, s.VS);
        nvrhi::hash_combine(seed, s.HS);
        nvrhi::hash_combine(seed, s.DS);
        nvrhi::hash_combine(seed, s.GS);
        nvrhi::hash_combine(seed, s.PS);
        nvrhi::hash_combine(seed, s.renderState);
        nvrhi::hash_combine(seed, s.shadingRateState);
        nvrhi::hash_combine(seed, s.bindingLayouts);
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::PipelineHitGroupDesc> {
    std::size_t operator()(
        nvrhi::rt::PipelineHitGroupDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.exportName);
        nvrhi::hash_combine(seed, s.closestHitShader);
        nvrhi::hash_combine(seed, s.anyHitShader);
        nvrhi::hash_combine(seed, s.intersectionShader);
        nvrhi::hash_combine(seed, s.bindingLayout);
        nvrhi::hash_combine(seed, s.isProceduralPrimitive);
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::PipelineShaderDesc> {
    std::size_t operator()(
        nvrhi::rt::PipelineShaderDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.exportName);
        nvrhi::hash_combine(seed, s.shader);
        nvrhi::hash_combine(seed, s.bindingLayout);
        return seed;
    }
};

template<typename T>
struct hash<std::vector<T>> {
    std::size_t operator()(std::vector<T> const& v) const noexcept
    {
        std::size_t seed = 0;
        for (const auto& elem : v) {
            nvrhi::hash_combine(seed, elem);
        }
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::PipelineDesc> {
    std::size_t operator()(nvrhi::rt::PipelineDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.shaders);
        nvrhi::hash_combine(seed, s.hitGroups);
        nvrhi::hash_combine(seed, s.globalBindingLayouts);
        nvrhi::hash_combine(seed, s.maxPayloadSize);
        nvrhi::hash_combine(seed, s.maxAttributeSize);
        nvrhi::hash_combine(seed, s.maxRecursionDepth);
        nvrhi::hash_combine(seed, s.hlslExtensionsUAV);
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::GeometryTriangles> {
    std::size_t operator()(nvrhi::rt::GeometryTriangles const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.indexBuffer);
        nvrhi::hash_combine(seed, s.vertexBuffer);
        nvrhi::hash_combine(seed, s.indexFormat);
        nvrhi::hash_combine(seed, s.vertexFormat);
        nvrhi::hash_combine(seed, s.indexOffset);
        nvrhi::hash_combine(seed, s.vertexOffset);
        nvrhi::hash_combine(seed, s.indexCount);
        nvrhi::hash_combine(seed, s.vertexCount);
        nvrhi::hash_combine(seed, s.vertexStride);
        nvrhi::hash_combine(seed, s.opacityMicromap);
        nvrhi::hash_combine(seed, s.ommIndexBuffer);
        nvrhi::hash_combine(seed, s.ommIndexBufferOffset);
        nvrhi::hash_combine(seed, s.ommIndexFormat);
        nvrhi::hash_combine(seed, s.pOmmUsageCounts);
        nvrhi::hash_combine(seed, s.numOmmUsageCounts);
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::GeometryDesc> {
    std::size_t operator()(nvrhi::rt::GeometryDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.geometryData.triangles);
        nvrhi::hash_combine(seed, s.useTransform);
        nvrhi::hash_combine(seed, s.flags);
        nvrhi::hash_combine(seed, s.geometryType);
        seed ^= std::hash<std::string_view>{}(std::string_view(
            reinterpret_cast<const char*>(s.transform),
            sizeof(nvrhi::rt::AffineTransform)));
        return seed;
    }
};

template<>
struct hash<nvrhi::rt::AccelStructDesc> {
    std::size_t operator()(nvrhi::rt::AccelStructDesc const& s) const noexcept
    {
        std::size_t seed = 0;
        nvrhi::hash_combine(seed, s.topLevelMaxInstances);
        nvrhi::hash_combine(seed, s.bottomLevelGeometries);
        nvrhi::hash_combine(seed, s.buildFlags);
        nvrhi::hash_combine(seed, s.debugName);
        nvrhi::hash_combine(seed, s.trackLiveness);
        nvrhi::hash_combine(seed, s.isTopLevel);
        nvrhi::hash_combine(seed, s.isVirtual);
        return seed;
    }
};

}  // namespace std
