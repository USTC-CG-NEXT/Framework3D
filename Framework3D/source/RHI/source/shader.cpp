#include "RHI/ShaderFactory/shader.hpp"

#include <Windows.h>
#include <dxcapi.h>
#include <wrl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "RHI/internal/resources.hpp"
#include "shaderCompiler.h"
#include "slang-com-ptr.h"
#include "slang.h"

using namespace Microsoft::WRL;

USTC_CG_NAMESPACE_OPEN_SCOPE
void const* Program::getBufferPointer() const
{
    return blob->getBufferPointer();
}

size_t Program::getBufferSize() const
{
    return blob->getBufferSize();
}

void ProgramDesc::set_path(const std::filesystem::path& path)
{
    this->path = path;
    update_last_write_time(path);
}

void ProgramDesc::set_shader_type(nvrhi::ShaderType shaderType)
{
    this->shaderType = shaderType;
}

void ProgramDesc::set_entry_name(const std::string& entry_name)
{
    this->entry_name = entry_name;
    update_last_write_time(path);
}
namespace fs = std::filesystem;

void ProgramDesc::update_last_write_time(const std::filesystem::path& path)
{
    if (fs::exists(path)) {
        auto possibly_newer_lastWriteTime = fs::last_write_time(path);
        if (possibly_newer_lastWriteTime > lastWriteTime) {
            lastWriteTime = possibly_newer_lastWriteTime;
        }
    }
    else {
        lastWriteTime = {};
    }
}

std::string ProgramDesc::get_profile() const
{
    switch (shaderType) {
        case nvrhi::ShaderType::None: break;
        case nvrhi::ShaderType::Compute: return "cs_6_5";
        case nvrhi::ShaderType::Vertex: return "vs_6_5";
        case nvrhi::ShaderType::Hull: return "hs_6_5";
        case nvrhi::ShaderType::Domain: return "ds_6_5";
        case nvrhi::ShaderType::Geometry: return "gs_6_5";
        case nvrhi::ShaderType::Pixel: return "ps_6_5";
        case nvrhi::ShaderType::Amplification: return "as_6_5";
        case nvrhi::ShaderType::Mesh: return "ms_6_5";
        case nvrhi::ShaderType::AllGraphics: return "lib_6_5";
        case nvrhi::ShaderType::RayGeneration: return "rg_6_5";
        case nvrhi::ShaderType::AnyHit: return "ah_6_5";
        case nvrhi::ShaderType::ClosestHit: return "ch_6_5";
        case nvrhi::ShaderType::Miss: return "ms_6_5";
        case nvrhi::ShaderType::Intersection: return "is_6_5";
        case nvrhi::ShaderType::Callable: return "cs_6_5";
        case nvrhi::ShaderType::AllRayTracing: return "lib_6_5";
        case nvrhi::ShaderType::All: return "lib_6_5";
    }

    // Default return value for cases not handled explicitly
    return "lib_6_5";
}

HRESULT DxcCompileHLSLToDXIL(
    const wchar_t* filename,
    const char* entryPoint,
    const char* profile,
    ComPtr<IDxcBlob>& ppResultBlob,
    std::string& error_string)
{
    ComPtr<IDxcCompiler3> pCompiler;
    ComPtr<IDxcUtils> pUtils;
    ComPtr<IDxcIncludeHandler> pIncludeHandler;
    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create DXC Compiler instance." << std::endl;
        return hr;
    }

    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create DXC Utils instance." << std::endl;
        return hr;
    }

    hr = pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create include handler." << std::endl;
        return hr;
    }

    std::ifstream shaderFile(filename, std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open()) {
        std::wcerr << L"Failed to open shader file." << std::endl;
        return E_FAIL;
    }

    ComPtr<IDxcBlobEncoding> sourceBlob;
    hr = pUtils->LoadFile(filename, nullptr, &sourceBlob);

    DxcBuffer sourceBuffer = {};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();

    wchar_t w_entryPoint[100];
    mbstowcs(w_entryPoint, entryPoint, strlen(entryPoint) + 1);  // Plus null

    wchar_t w_profile[100];
    mbstowcs(w_profile, profile, strlen(profile) + 1);  // Plus null

    const wchar_t* args[] = {
        filename,
        L"-T",
        w_profile,
        L"-Zi",              // Enable debug information.
        L"-Qstrip_reflect",  // Strip reflection data for smaller binaries.
        L"-enable-16bit-types",
        DXC_ARG_OPTIMIZATION_LEVEL3,
    };

    ComPtr<IDxcResult> pResults;
    hr = pCompiler->Compile(
        &sourceBuffer,
        args,
        _countof(args),
        pIncludeHandler.Get(),
        IID_PPV_ARGS(&pResults));
    if (FAILED(hr)) {
        std::wcerr << L"Compilation failed." << std::endl;
        return hr;
    }

    ComPtr<IDxcBlobUtf8> pErrors;
    hr = pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    if (SUCCEEDED(hr) && pErrors != nullptr && pErrors->GetStringLength() > 0) {
        std::wcerr << L"Warnings and Errors:" << std::endl;
        std::wcerr << pErrors->GetStringPointer() << std::endl;
        error_string = pErrors->GetStringPointer();
    }

    hr = pResults->GetResult(&ppResultBlob);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to get compiled shader." << std::endl;
        return hr;
    }

    return S_OK;
}

Slang::ComPtr<slang::IGlobalSession> globalSession;

Slang::ComPtr<slang::IGlobalSession> createGlobal()
{
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    slang::createGlobalSession(globalSession.writeRef());

    SlangShaderCompiler::addHLSLPrelude(globalSession);

    return globalSession;
}

static nvrhi::ResourceType convertBindingTypeToResourceType(
    slang::BindingType bindingType)
{
    using namespace nvrhi;
    using namespace slang;
    switch (bindingType) {
        case BindingType::Sampler: return ResourceType::Sampler;
        case BindingType::Texture:
        case BindingType::CombinedTextureSampler:
        case BindingType::InputRenderTarget: return ResourceType::Texture_SRV;
        case BindingType::MutableTexture: return ResourceType::Texture_UAV;
        case BindingType::TypedBuffer:
        case BindingType::MutableTypedBuffer:
            return ResourceType::TypedBuffer_SRV;
        case BindingType::RawBuffer: return ResourceType::RawBuffer_SRV;
        case BindingType::MutableRawBuffer: return ResourceType::RawBuffer_UAV;
        case BindingType::ConstantBuffer:
        case BindingType::ParameterBlock: return ResourceType::ConstantBuffer;
        case BindingType::RayTracingAccelerationStructure:
            return ResourceType::RayTracingAccelStruct;
        case BindingType::PushConstant: return ResourceType::PushConstants;
        case BindingType::InlineUniformData:
        case BindingType::VaryingInput:
        case BindingType::VaryingOutput:
        case BindingType::ExistentialValue:
        case BindingType::MutableFlag:
        case BindingType::BaseMask:
        case BindingType::ExtMask:
        case BindingType::Unknown:
        default: return ResourceType::None;
    }
}

nvrhi::BindingLayoutDescVector shader_reflect(
    SlangCompileRequest* request,
    nvrhi::ShaderType shader_type)
{
    slang::ShaderReflection* programReflection =
        slang::ShaderReflection::get(request);

    // slang::EntryPointReflection* entryPoint =
    //     programReflection->findEntryPointByName(entryPointName);
    auto parameterCount = programReflection->getParameterCount();
    // auto parameterCount = entryPoint->getParameterCount();
    nvrhi::BindingLayoutDescVector ret;

    for (int pp = 0; pp < parameterCount; ++pp) {
        slang::VariableLayoutReflection* parameter =
            programReflection->getParameterByIndex(pp);

        slang::TypeLayoutReflection* typeLayout = parameter->getTypeLayout();
        auto categoryCount = parameter->getCategoryCount();
        assert(categoryCount == 1);
        auto category =
            SlangParameterCategory(parameter->getCategoryByIndex(0));

        auto index = parameter->getBindingIndex();
        auto space = parameter->getBindingSpace(category);

        auto bindingRangeCount = typeLayout->getBindingRangeCount();
        assert(bindingRangeCount == 1);
        slang::BindingType type = typeLayout->getBindingRangeType(0);

        nvrhi::BindingLayoutItem item;

        item.type = convertBindingTypeToResourceType(type);
        item.slot = index;

        if (ret.size() < space + 1) {
            ret.resize(space + 1);
        }

        ret[space].addItem(item);
        ret[space].visibility = shader_type;
    }

    return ret;
}

// Function to convert ShaderType to SlangStage
SlangStage ConvertShaderTypeToSlangStage(nvrhi::ShaderType shaderType)
{
    using namespace nvrhi;
    switch (shaderType) {
        case ShaderType::Vertex: return SLANG_STAGE_VERTEX;
        case ShaderType::Hull: return SLANG_STAGE_HULL;
        case ShaderType::Domain: return SLANG_STAGE_DOMAIN;
        case ShaderType::Geometry: return SLANG_STAGE_GEOMETRY;
        case ShaderType::Pixel:
            return SLANG_STAGE_FRAGMENT;  // alias for SLANG_STAGE_PIXEL
        case ShaderType::Amplification: return SLANG_STAGE_AMPLIFICATION;
        case ShaderType::Mesh: return SLANG_STAGE_MESH;
        case ShaderType::Compute: return SLANG_STAGE_COMPUTE;
        case ShaderType::RayGeneration: return SLANG_STAGE_RAY_GENERATION;
        case ShaderType::AnyHit: return SLANG_STAGE_ANY_HIT;
        case ShaderType::ClosestHit: return SLANG_STAGE_CLOSEST_HIT;
        case ShaderType::Miss: return SLANG_STAGE_MISS;
        case ShaderType::Intersection: return SLANG_STAGE_INTERSECTION;
        case ShaderType::Callable: return SLANG_STAGE_CALLABLE;
        default: return SLANG_STAGE_NONE;
    }
}

nvrhi::BindingLayoutDescVector mergeBindingLayoutDescVectors(
    const nvrhi::BindingLayoutDescVector& vec1,
    const nvrhi::BindingLayoutDescVector& vec2)
{
    nvrhi::BindingLayoutDescVector result;
    size_t maxSize = std::max(vec1.size(), vec2.size());

    for (size_t i = 0; i < maxSize; ++i) {
        BindingLayoutDesc mergedDesc;

        if (i < vec1.size()) {
            mergedDesc = vec1[i];
        }
        else {
            mergedDesc = BindingLayoutDesc();
        }

        if (i < vec2.size()) {
            const BindingLayoutDesc& desc2 = vec2[i];
            mergedDesc.visibility = mergedDesc.visibility | desc2.visibility;
            for (int j = 0; j < desc2.bindings.size(); ++j) {
                mergedDesc.bindings.push_back(desc2.bindings[j]);
            }
        }

        result.push_back(mergedDesc);
    }

    return result;
}
nvrhi::ShaderHandle ShaderFactory::compile_shader(
    const std::string& entryName,
    nvrhi::ShaderType shader_type,
    std::filesystem::path shader_path,
    nvrhi::BindingLayoutDescVector& binding_layout_desc,
    std::string& error_string,
    const std::vector<ShaderMacro>& macro_defines,

    const std::string& source_code,
    bool absolute)
{
    ProgramDesc shader_compile_desc;

    if (!absolute && shader_path != "") {
        shader_path = std::filesystem::path(shader_search_path) / shader_path;
    }
    shader_compile_desc.set_entry_name(entryName);
    if (shader_path != "") {
        shader_compile_desc.set_path(shader_path);
    }
    for (const auto& macro_define : macro_defines) {
        shader_compile_desc.define(macro_define.name, macro_define.definition);
    }
    shader_compile_desc.shaderType = shader_type;
    shader_compile_desc.source_code = source_code;
    auto shader_compiled = createProgram(shader_compile_desc);

    if (!shader_compiled->get_error_string().empty()) {
        error_string = shader_compiled->get_error_string();
        shader_compiled = nullptr;
        return nullptr;
    }

    nvrhi::ShaderDesc desc;
    desc.shaderType = shader_compile_desc.shaderType;
    desc.entryName = entryName;
    desc.debugName = std::to_string(
        reinterpret_cast<long long>(shader_compiled->getBufferPointer()));

    binding_layout_desc = shader_compiled->get_binding_layout_descs();

    auto compute_shader = device->createShader(
        desc,
        shader_compiled->getBufferPointer(),
        shader_compiled->getBufferSize());
    shader_compiled = nullptr;

    return compute_shader;
}

void ShaderFactory::SlangCompile(
    const std::filesystem::path& path,
    const std::string& sourceCode,
    const char* entryPoint,
    nvrhi::ShaderType shaderType,
    const char* profile,
    const std::vector<ShaderMacro>& defines,
    nvrhi::BindingLayoutDescVector& shader_reflection,
    Slang::ComPtr<ISlangBlob>& ppResultBlob,
    std::string& error_string,
    SlangCompileTarget target) const
{
    auto stage = ConvertShaderTypeToSlangStage(shaderType);

    if (!globalSession) {
        globalSession = createGlobal();
    }

    Slang::ComPtr<slang::ISession> pSlangSession;
    slang::SessionDesc sessionDesc;
    std::vector<std::string> searchPaths = { shader_search_path + "/shaders/" };
    std::vector<const char*> slangSearchPaths;
    for (auto& path : searchPaths) {
        slangSearchPaths.push_back(path.data());
    }
    sessionDesc.searchPaths = slangSearchPaths.data();
    sessionDesc.searchPathCount = (SlangInt)slangSearchPaths.size();
    auto result =
        globalSession->createSession(sessionDesc, pSlangSession.writeRef());
    assert(result == SLANG_OK);

    SlangCompileRequest* slangRequest = nullptr;
    result = pSlangSession->createCompileRequest(&slangRequest);

    int targetIndex = slangRequest->addCodeGenTarget(target);
    slangRequest->setTargetFlags(
        targetIndex,
        (target != SLANG_SPIRV) ? SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM
                                : kDefaultTargetFlags);

    int translationUnitIndex =
        slangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    auto profile_id = globalSession->findProfile(profile);
    slangRequest->setTargetProfile(targetIndex, profile_id);

    if (!sourceCode.empty()) {
        slangRequest->addTranslationUnitSourceString(
            translationUnitIndex, "shader", sourceCode.c_str());
    }
    else {
        slangRequest->addTranslationUnitSourceFile(
            translationUnitIndex, path.generic_string().c_str());
    }

    for (const auto& define : defines) {
        slangRequest->addPreprocessorDefine(
            define.name.c_str(), define.definition.c_str());
    }

    int entryPointIndex = -1;
    if (entryPoint && *entryPoint) {
        entryPointIndex = slangRequest->addEntryPoint(
            translationUnitIndex, entryPoint, stage);
    }

    const SlangResult compileRes = slangRequest->compile();

    if (SLANG_FAILED(compileRes)) {
        if (auto diagnostics = slangRequest->getDiagnosticOutput()) {
            error_string = diagnostics;
        }
        spDestroyCompileRequest(slangRequest);
        return;
    }

    shader_reflection = shader_reflect(slangRequest, shaderType);
    result = slangRequest->getEntryPointCodeBlob(
        entryPointIndex, targetIndex, ppResultBlob.writeRef());
    assert(result == SLANG_OK);

    spDestroyCompileRequest(slangRequest);
}

ProgramHandle ShaderFactory::createProgram(const ProgramDesc& desc)
{
    ProgramHandle ret = ProgramHandle::Create(new Program);

    SlangCompileTarget target =
        (rhi::get_backend() == nvrhi::GraphicsAPI::VULKAN) ? SLANG_SPIRV
                                                           : SLANG_DXIL;

    SlangCompile(
        desc.path,
        desc.source_code,
        desc.entry_name.c_str(),
        desc.shaderType,
        desc.get_profile().c_str(),
        desc.macros,
        ret->binding_layout_,
        ret->blob,
        ret->error_string,
        target);

    return ret;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE