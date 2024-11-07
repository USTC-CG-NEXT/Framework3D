#pragma once
#include <filesystem>

#include "RHI/internal/resources.hpp"
#include "RHI/rhi.hpp"
#include "rhi/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class RHI_API ShaderFactory {
   public:
    ShaderFactory() : device(RHI::get_device())
    {
    }

    nvrhi::ShaderHandle compile_shader(
        const std::string& entryName,
        nvrhi::ShaderType shader_type,
        std::filesystem::path shader_path,
        nvrhi::BindingLayoutDescVector& binding_layout_desc,
        std::string& error_string,
        const std::vector<ShaderMacro>& macro_defines,
        const std::string& source_code,
        bool absolute = false);

    ProgramHandle createProgram(const ProgramDesc& desc) const;

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

    std::string shader_search_path;
    nvrhi::IDevice* device;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE