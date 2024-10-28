#pragma once
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "RHI/rhi.hpp"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class ShaderFactory {
   public:
    ShaderFactory() : device(rhi::get_device())
    {
        shader_search_path =
            "C:\\Users\\pengfei\\WorkSpace\\USTC_CG_"
            "24\\Framework3D\\source\\GUI\\source";
    }

    nvrhi::ShaderHandle USTC_CG_API compile_shader(
        const std::string& entryName,
        nvrhi::ShaderType shader_type,
        std::filesystem::path shader_path,
        nvrhi::BindingLayoutDescVector& binding_layout_desc,
        std::string& error_string,
        const std::vector<ShaderMacro>& macro_defines,
        const std::string& source_code,
        bool absolute = false);

   private:
    void SlangCompile(
        const std::filesystem::path& path,
        const std::string& sourceCode,
        const char* entryPoint,
        nvrhi::ShaderType shaderType,
        const char* profile,
        const std::vector<ShaderMacro>& defines,
        nvrhi::BindingLayoutDescVector& shader_reflection,
        Slang::ComPtr<ISlangBlob>& ppResultBlob,
        std::string& error_string,
        SlangCompileTarget target) const;

    ProgramHandle createProgram(const ProgramDesc& desc);

    std::string shader_search_path;
    nvrhi::IDevice* device;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE