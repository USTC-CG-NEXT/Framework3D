#pragma once
#include <nvrhi/nvrhi.h>

#include <filesystem>
#include <map>

#include "RHI/api.h"
#include "RHI/internal/resources.hpp"
#include "RHI/rhi.hpp"
#include "shader_reflection.hpp"

namespace USTC_CG {
class ResourceAllocator;
}

USTC_CG_NAMESPACE_OPEN_SCOPE
class RHI_API ShaderFactory {
   public:
    ShaderFactory() : device(RHI::get_device()), resource_allocator(nullptr)
    {
    }

    ShaderFactory(ResourceAllocator* resource_allocator)
        : device(RHI::get_device()),
          resource_allocator(resource_allocator)
    {
    }

    ShaderHandle compile_shader(
        const std::string& entryName,
        nvrhi::ShaderType shader_type,
        std::filesystem::path shader_path,
        ShaderReflectionInfo& reflection_info,
        std::string& error_string,
        const std::vector<ShaderMacro>& macro_defines = {},
        const std::string& source_code = {});

    ProgramHandle compile_cpu_executable(
        const std::string& entryName,
        nvrhi::ShaderType shader_type,
        std::filesystem::path shader_path,
        ShaderReflectionInfo& reflection_info,
        std::string& error_string,
        const std::vector<ShaderMacro>& macro_defines = {},
        const std::string& source_code = {});

    ProgramHandle createProgram(const ProgramDesc& desc) const;

    static void set_search_path(const std::string& string)
    {
        shader_search_path = string;
    }

   private:
    void SlangCompile(
        const std::filesystem::path& path,
        const std::string& sourceCode,
        const char* entryPoint,
        nvrhi::ShaderType shaderType,
        const char* profile,
        const std::vector<ShaderMacro>& defines,
        ShaderReflectionInfo& shader_reflection,
        Slang::ComPtr<ISlangBlob>& ppResultBlob,
        Slang::ComPtr<ISlangSharedLibrary>& ppSharedLirary,
        std::string& error_string,
        SlangCompileTarget target) const;

    static void populate_vk_options(
        std::vector<slang::CompilerOptionEntry>& vk_compiler_options);
    void modify_vulkan_binding_shift(nvrhi::BindingLayoutItem& item) const;
    ShaderReflectionInfo shader_reflect(
        slang::IComponentType* component,
        nvrhi::ShaderType shader_type) const;

    static constexpr int SRV_OFFSET = 0;
    static constexpr int SAMPLER_OFFSET = 128;
    static constexpr int CONSTANT_BUFFER_OFFSET = 256;
    static constexpr int UAV_OFFSET = 384;

    static std::string shader_search_path;
    nvrhi::IDevice* device;
    ResourceAllocator* resource_allocator;
    friend class ProgramDesc;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE