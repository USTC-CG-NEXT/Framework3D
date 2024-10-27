#pragma once

#include "RHI/resource_allocator.hpp"
#include "RHI/rhi.hpp"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class ShaderFactory {
   public:
    ShaderFactory() : device(get_device().Get())
    {
        shader_search_path =
            "C:\\Users\\Jerry\\WorkSpace\\USTC_CG_"
            "NEXT\\Framework3D\\source\\GUI\\window";
    }

    nvrhi::ShaderHandle USTC_CG_API compile_shader(
        const std::string& entryName,
        nvrhi::ShaderType shader_type,
        std::filesystem::path shader_path,
        nvrhi::BindingLayoutDescVector& binding_layout_desc,
        std::string& error_string,
        const std::vector<ShaderMacro>& macro_defines,
        bool nvapi_support = false,
        bool absolute = false);

   private:
    void SlangCompileHLSLToDXIL(
        const char* filename,
        const char* entryPoint,
        nvrhi::ShaderType shaderType,
        const char* profile,
        const std::vector<ShaderMacro>& defines,  // List of macro defines
        nvrhi::BindingLayoutDescVector& shader_reflection,
        Slang::ComPtr<ISlangBlob>& ppResultBlob,
        std::string& error_string,
        bool nvapi_support);
    void SlangCompileHLSLToSPIRV(
        const char* filename,
        const char* entryPoint,
        nvrhi::ShaderType shaderType,
        const char* profile,
        const std::vector<ShaderMacro>& defines,  // List of macro defines
        nvrhi::BindingLayoutDescVector& shader_reflection,
        Slang::ComPtr<ISlangBlob>& ppResultBlob,
        std::string& error_string,
        bool nvapi_support);

    ProgramHandle createProgram(const ProgramDesc& desc);

    std::string shader_search_path;
    nvrhi::IDevice* device;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE