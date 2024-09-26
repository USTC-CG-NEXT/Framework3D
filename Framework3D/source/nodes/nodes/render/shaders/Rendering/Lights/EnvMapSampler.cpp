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
#include "EnvMapSampler.h"

#include "Core/Error.h"
#include "Core/Pass/ComputePass.h"
#include "Utils/Logging/Logging.h"

namespace Falcor {
namespace {
    const char kShaderFilenameSetup[] =
        "Rendering/Lights/EnvMapSamplerSetup.cs.slang";

    // The defaults are 512x512 @ 64spp in the resampling step.
    const uint32_t kDefaultDimension = 512;
    const uint32_t kDefaultSpp = 64;
}  // namespace

EnvMapSampler::EnvMapSampler(ref<Device> pDevice, ref<EnvMap> pEnvMap)
    : mpDevice(pDevice),
      mpEnvMap(pEnvMap)
{
    FALCOR_ASSERT(pEnvMap);

    // Create compute program for the setup phase.
    mpSetupPass = ComputePass::create(mpDevice, kShaderFilenameSetup, "main");

    // Create sampler.
    SamplerDesc samplerDesc;
    samplerDesc.setAllFilters(false);
    samplerDesc.setAllAddressModes(SamplerAddressMode::Clamp);
    mpImportanceSampler = mpDevice->createSampler(samplerDesc);

    // Create hierarchical importance map for sampling.
    if (!createImportanceMap(
            mpDevice->getRenderContext(), kDefaultDimension, kDefaultSpp)) {
        FALCOR_THROW("Failed to create importance map");
    }
}

void EnvMapSampler::bindShaderData(const ShaderVar& var) const
{
    FALCOR_ASSERT(var.isValid());

    // Set variables.
    float2 invDim = 1.f / float2(
                              mpImportanceMap->getDesc().width,
                              mpImportanceMap->getDesc().height);
    var["importanceBaseMip"] =
        mpImportanceMap->getDesc().mipLevels - 1;  // The base mip is 1x1 texels
    var["importanceInvDim"] = invDim;

    // Bind resources.
    var["importanceMap"] = mpImportanceMap;
    var["importanceSampler"] = mpImportanceSampler;
}

bool EnvMapSampler::createImportanceMap(
    RenderContext* pRenderContext,
    uint32_t dimension,
    uint32_t samples)
{
    FALCOR_ASSERT(isPowerOf2(dimension));
    FALCOR_ASSERT(isPowerOf2(samples));

    // We create log2(N)+1 mips from NxN...1x1 texels resolution.
    uint32_t mips = std::log2(dimension) + 1;
    FALCOR_ASSERT((1u << (mips - 1)) == dimension);
    FALCOR_ASSERT(
        mips > 1 &&
        mips <=
            12);  // Shader constant limits max resolution, increase if needed.

    // Create importance map. We have to set the RTV flag to be able to use
    // generateMips().
    nvrhi::TextureDesc textureDesc;
    textureDesc.width = dimension;
    textureDesc.height = dimension;
    textureDesc.mipLevels = mips;
    textureDesc.format = nvrhi::Format::R32_FLOAT;
    textureDesc.isRenderTarget = true;
    textureDesc.isUAV = true;
    textureDesc.isShaderResource = true;
    mpImportanceMap = mpDevice->createTexture(textureDesc);

    FALCOR_ASSERT(mpImportanceMap);

    auto var = mpSetupPass->getRootVar();
    var["gEnvMap"] = mpEnvMap->getEnvMap();
    var["gEnvSampler"] = mpEnvMap->getEnvSampler();
    var["gImportanceMap"] = mpImportanceMap;

    uint32_t samplesX = std::max(1u, (uint32_t)std::sqrt(samples));
    uint32_t samplesY = samples / samplesX;
    FALCOR_ASSERT(samples == samplesX * samplesY);

    var["CB"]["outputDim"] = uint2(dimension);
    var["CB"]["outputDimInSamples"] =
        uint2(dimension * samplesX, dimension * samplesY);
    var["CB"]["numSamples"] = uint2(samplesX, samplesY);
    var["CB"]["invSamples"] = 1.f / (samplesX * samplesY);

    // Execute setup pass to compute the square importance map (base mip).
    mpSetupPass->execute(pRenderContext, dimension, dimension);

    // Populate mip hierarchy. We rely on the default mip generation for this.
    USTC_CG::logging("Env map mips generation not implemented");
    //mpImportanceMap->generateMips(pRenderContext);

    return true;
}

}  // namespace Falcor
