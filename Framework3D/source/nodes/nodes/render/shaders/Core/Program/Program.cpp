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
#include "Program.h"

#include <slang.h>

#include <set>

#include "Core/Error.h"
#include "Core/ObjectPython.h"
#include "Core/Platform/OS.h"
#include "ProgramManager.h"
#include "ProgramVars.h"
#include "Utils/CryptoUtils.h"
#include "Utils/Logger.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "Utils/StringUtils.h"
#include "Utils/Timing/CpuTimer.h"

namespace Falcor {

void ProgramDesc::finalize()
{
    uint32_t globalIndex = 0;
    for (auto& entryPointGroup : entryPointGroups)
        for (auto& entryPoint : entryPointGroup.entryPoints)
            entryPoint.globalIndex = globalIndex++;
}

Program::Program(
    ref<Device> pDevice,
    ProgramDesc desc,
    DefineList defineList)
    : mpDevice(std::move(pDevice)),
      mDesc(std::move(desc)),
      mDefineList(std::move(defineList)),
      mTypeConformanceList(mDesc.typeConformances)
{
    mDesc.finalize();

    if (mDesc.hasEntryPoint(ShaderType::RayGeneration)) {
        if (desc.maxTraceRecursionDepth == uint32_t(-1))
            FALCOR_THROW(
                "Can't create a raytacing program without specifying maximum "
                "trace recursion depth");
        if (desc.maxPayloadSize == uint32_t(-1))
            FALCOR_THROW(
                "Can't create a raytacing program without specifying maximum "
                "ray payload size");
    }

    validateEntryPoints();

    mpDevice->getProgramManager()->registerProgramForReload(this);
}

Program::~Program()
{
    mpDevice->getProgramManager()->unregisterProgramForReload(this);

    // Invalidate program versions.
    for (auto& version : mProgramVersions)
        version.second->mpProgram = nullptr;
}

void Program::validateEntryPoints() const
{
    // Check that all exported entry point names are unique for each shader
    // type. They don't necessarily have to be, but it could be an indication of
    // the program not created correctly.
    using NameTypePair = std::pair<std::string, ShaderType>;
    std::set<NameTypePair> entryPointNamesAndTypes;
    for (const auto& group : mDesc.entryPointGroups) {
        for (const auto& e : group.entryPoints) {
            if (!entryPointNamesAndTypes
                     .insert(NameTypePair(e.exportName, e.type))
                     .second) {
                logWarning(
                    "Duplicate program entry points '{}' of type '{}'.",
                    e.exportName,
                    e.type);
            }
        }
    }
}

std::string Program::getProgramDescString() const
{
    std::string desc;

    for (const auto& shaderModule : mDesc.shaderModules) {
        for (const auto& source : shaderModule.sources) {
            switch (source.type) {
                case ProgramDesc::ShaderSource::Type::File:
                    desc += source.path.string();
                    break;
                case ProgramDesc::ShaderSource::Type::String:
                    desc += "<string>";
                    break;
                default: FALCOR_UNREACHABLE();
            }
            desc += " ";
        }
    }

    desc += "(";
    size_t entryPointIndex = 0;
    for (const auto& entryPointGroup : mDesc.entryPointGroups) {
        for (const auto& entryPoint : entryPointGroup.entryPoints) {
            if (entryPointIndex++ > 0)
                desc += ", ";
            desc += entryPoint.exportName;
        }
    }
    desc += ")";

    return desc;
}

bool Program::addDefine(const std::string& name, const std::string& value)
{
    // Make sure that it doesn't exist already
    if (mDefineList.find(name) != mDefineList.end()) {
        if (mDefineList[name] == value) {
            // Same define
            return false;
        }
    }
    markDirty();
    mDefineList[name] = value;
    return true;
}

bool Program::addDefines(const DefineList& dl)
{
    bool dirty = false;
    for (auto it : dl) {
        if (addDefine(it.first, it.second)) {
            dirty = true;
        }
    }
    return dirty;
}

bool Program::removeDefine(const std::string& name)
{
    if (mDefineList.find(name) != mDefineList.end()) {
        markDirty();
        mDefineList.erase(name);
        return true;
    }
    return false;
}

bool Program::removeDefines(const DefineList& dl)
{
    bool dirty = false;
    for (auto it : dl) {
        if (removeDefine(it.first)) {
            dirty = true;
        }
    }
    return dirty;
}

bool Program::removeDefines(size_t pos, size_t len, const std::string& str)
{
    bool dirty = false;
    for (auto it = mDefineList.cbegin(); it != mDefineList.cend();) {
        if (pos < it->first.length() && it->first.compare(pos, len, str) == 0) {
            markDirty();
            it = mDefineList.erase(it);
            dirty = true;
        }
        else {
            ++it;
        }
    }
    return dirty;
}

bool Program::setDefines(const DefineList& dl)
{
    if (dl != mDefineList) {
        markDirty();
        mDefineList = dl;
        return true;
    }
    return false;
}

bool Program::addTypeConformance(
    const std::string& typeName,
    const std::string interfaceType,
    uint32_t id)
{
    TypeConformance conformance = TypeConformance(typeName, interfaceType);
    if (mTypeConformanceList.find(conformance) == mTypeConformanceList.end()) {
        markDirty();
        mTypeConformanceList.add(typeName, interfaceType, id);
        return true;
    }
    return false;
}

bool Program::removeTypeConformance(
    const std::string& typeName,
    const std::string interfaceType)
{
    TypeConformance conformance = TypeConformance(typeName, interfaceType);
    if (mTypeConformanceList.find(conformance) != mTypeConformanceList.end()) {
        markDirty();
        mTypeConformanceList.remove(typeName, interfaceType);
        return true;
    }
    return false;
}

bool Program::setTypeConformances(const TypeConformanceList& conformances)
{
    if (conformances != mTypeConformanceList) {
        markDirty();
        mTypeConformanceList = conformances;
        return true;
    }
    return false;
}

bool Program::checkIfFilesChanged()
{
    if (mpActiveVersion == nullptr) {
        // We never linked, so nothing really changed
        return false;
    }

    // Have any of the files we depend on changed?
    for (auto& entry : mFileTimeMap) {
        auto& path = entry.first;
        auto& modifiedTime = entry.second;

        if (modifiedTime != getFileModifiedTime(path)) {
            return true;
        }
    }
    return false;
}

const ref<const ProgramVersion>& Program::getActiveVersion() const
{
    if (mLinkRequired) {
        const auto& it = mProgramVersions.find(
            ProgramVersionKey{ mDefineList, mTypeConformanceList });
        if (it == mProgramVersions.end()) {
            // Note that link() updates mActiveProgram only if the operation was
            // successful. On error we get false, and mActiveProgram points to
            // the last successfully compiled version.
            if (link() == false) {
                FALCOR_THROW("Program linkage failed");
            }
            else {
                mProgramVersions[ProgramVersionKey{
                    mDefineList, mTypeConformanceList }] = mpActiveVersion;
            }
        }
        else {
            mpActiveVersion = it->second;
        }
        mLinkRequired = false;
    }
    FALCOR_ASSERT(mpActiveVersion);
    return mpActiveVersion;
}

bool Program::link() const
{
    while (1) {
        // Create the program
        std::string log;
        auto pVersion =
            mpDevice->getProgramManager()->createProgramVersion(*this, log);

        if (pVersion == nullptr) {
            std::string msg = "Failed to link program:\n" +
                              getProgramDescString() + "\n\n" + log;
            bool showMessageBox = is_set(
                getErrorDiagnosticFlags(),
                ErrorDiagnosticFlags::ShowMessageBoxOnError);
            if (showMessageBox && reportErrorAndAllowRetry(msg))
                continue;
            FALCOR_THROW(msg);
        }
        else {
            if (!log.empty()) {
                logWarning(
                    "Warnings in program:\n{}\n{}",
                    getProgramDescString(),
                    log);
            }

            mpActiveVersion = pVersion;
            return true;
        }
    }
}

void Program::reset()
{
    mpActiveVersion = nullptr;
    mProgramVersions.clear();
    mFileTimeMap.clear();
    mLinkRequired = true;
}
}  // namespace Falcor
