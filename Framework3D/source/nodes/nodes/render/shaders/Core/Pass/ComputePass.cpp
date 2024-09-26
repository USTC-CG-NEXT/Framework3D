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
#include "ComputePass.h"


#include "Core/API/ComputeContext.h"
#include "Utils/Math/Common.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
ComputePass::ComputePass(nvrhi::DeviceHandle pDevice, const ProgramDesc& desc, const DefineList& defines, bool createVars) : mpDevice(pDevice)
{
    auto pProg = Program::create(mpDevice, desc, defines);
    mpState = ComputeState::create(mpDevice);
    mpState->setProgram(pProg);
    if (createVars)
        mpVars = ProgramVars::create(mpDevice, pProg.get());
    FALCOR_ASSERT(pProg && mpState && (!createVars || mpVars));
}

ref<ComputePass> ComputePass::create(
    nvrhi::DeviceHandle pDevice,
    const std::filesystem::path& path,
    const std::string& csEntry,
    const DefineList& defines,
    bool createVars
)
{
    ProgramDesc desc;
    desc.addShaderLibrary(path).csEntry(csEntry);
    return create(pDevice, desc, defines, createVars);
}

ref<ComputePass> ComputePass::create(nvrhi::DeviceHandle pDevice, const ProgramDesc& desc, const DefineList& defines, bool createVars)
{
    return ref<ComputePass>(new ComputePass(pDevice, desc, defines, createVars));
}

void ComputePass::execute(ComputeContext* pContext, uint32_t nThreadX, uint32_t nThreadY, uint32_t nThreadZ)
{
    FALCOR_ASSERT(mpVars);
    uint3 threadGroupSize = mpState->getProgram()->getReflector()->getThreadGroupSize();
    uint3 groups = div_round_up(uint3(nThreadX, nThreadY, nThreadZ), threadGroupSize);
    pContext->dispatch(mpState.get(), mpVars.get(), groups);
}

void ComputePass::executeIndirect(ComputeContext* pContext, const Buffer* pArgBuffer, uint64_t argBufferOffset)
{
    FALCOR_ASSERT(mpVars);
    pContext->dispatchIndirect(mpState.get(), mpVars.get(), pArgBuffer, argBufferOffset);
}

void ComputePass::addDefine(const std::string& name, const std::string& value, bool updateVars)
{
    nvrhi::ComputeState state;
    state.
    mpState->getProgram()->addDefine(name, value);
    if (updateVars)
        mpVars = ProgramVars::create(mpDevice, mpState->getProgram().get());
}

void ComputePass::removeDefine(const std::string& name, bool updateVars)
{
    mpState->getProgram()->removeDefine(name);
    if (updateVars)
        mpVars = ProgramVars::create(mpDevice, mpState->getProgram().get());
}

void ComputePass::setVars(const ref<ProgramVars>& pVars)
{
    mpVars = pVars ? pVars : ProgramVars::create(mpDevice, mpState->getProgram().get());
    FALCOR_ASSERT(mpVars);
}

} // namespace Falcor
