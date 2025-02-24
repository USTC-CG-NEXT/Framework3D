#pragma once

#define SLANG_PRELUDE_NAMESPACE CPPPrelude

#include <RHI/ShaderFactory/shader_reflection.hpp>
#include <filesystem>
#include <map>

#include "RHI/api.h"
#include "map.h"
#include "nvrhi/nvrhi.h"
#include "nvrhi_patch.hpp"
#include "slang-com-ptr.h"
#include "slang-cpp-prelude.h"
#include "slang-cpp-types.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
class ShaderReflectionInfo;
struct Program;
struct ProgramDesc;
#define USING_NVRHI_SYMBOL(RESOURCE) \
    using nvrhi::RESOURCE##Desc;     \
    using nvrhi::RESOURCE##Handle;

#define USING_NVRHI_RT_SYMBOL(RESOURCE) \
    using nvrhi::rt::RESOURCE##Desc;    \
    using nvrhi::rt::RESOURCE##Handle;

#define NVRHI_RESOURCE_LIST                                                   \
    Texture, Sampler, Framebuffer, Shader, Buffer, BindingLayout, BindingSet, \
        CommandList, StagingTexture, ComputePipeline, GraphicsPipeline
#define NVRHI_RT_RESOURCE_LIST Pipeline, AccelStruct
#define RESOURCE_LIST          NVRHI_RESOURCE_LIST, NVRHI_RT_RESOURCE_LIST, Program

MACRO_MAP(USING_NVRHI_SYMBOL, NVRHI_RESOURCE_LIST);
MACRO_MAP(USING_NVRHI_RT_SYMBOL, NVRHI_RT_RESOURCE_LIST);

struct RHI_API ShaderMacro {
    std::string name;
    std::string definition;

    ShaderMacro(const std::string& _name, const std::string& _definition)
        : name(_name),
          definition(_definition)
    {
    }

    friend bool operator==(const ShaderMacro& lhs, const ShaderMacro& rhs)
    {
        return lhs.name == rhs.name && lhs.definition == rhs.definition;
    }

    friend bool operator!=(const ShaderMacro& lhs, const ShaderMacro& rhs)
    {
        return !(lhs == rhs);
    }
};

struct RHI_API ProgramDesc {
    friend bool operator==(const ProgramDesc& lhs, const ProgramDesc& rhs)
    {
        return lhs.path == rhs.path && lhs.entry_name == rhs.entry_name &&
               lhs.lastWriteTime == rhs.lastWriteTime &&
               lhs.shaderType == rhs.shaderType &&
               lhs.nvapi_support == rhs.nvapi_support &&
               lhs.macros == rhs.macros;
    }

    friend bool operator!=(const ProgramDesc& lhs, const ProgramDesc& rhs)
    {
        return !(lhs == rhs);
    }

    void define(std::string macro, std::string value)
    {
        macros.push_back(ShaderMacro(macro, value));
    }

    void define(const std::vector<ShaderMacro>& _macros)
    {
        macros.insert(macros.end(), _macros.begin(), _macros.end());
    }

    ProgramDesc& set_path(const std::filesystem::path& path);
    ProgramDesc& set_shader_type(nvrhi::ShaderType shaderType);
    ProgramDesc& set_entry_name(const std::string& entry_name);

    nvrhi::ShaderType shaderType;
    bool nvapi_support = false;

   private:
    void update_last_write_time(const std::filesystem::path& path);
    std::vector<ShaderMacro> macros;
    std::string get_profile() const;
    std::filesystem::path path;
    std::string source_code;
    std::filesystem::file_time_type lastWriteTime;
    std::string entry_name;

    friend class ShaderFactory;
    friend class Program;
    friend class std::hash<ProgramDesc>;
};

using ProgramHandle = nvrhi::RefCountPtr<Program>;

class RHI_API IProgram : public nvrhi::IResource {
   public:
    virtual ProgramDesc get_desc() const = 0;
    virtual nvrhi::ShaderDesc get_shader_desc() const = 0;
    virtual void const* getBufferPointer() const = 0;
    virtual size_t getBufferSize() const = 0;
    virtual const std::string& get_error_string() const = 0;
    virtual const ShaderReflectionInfo& get_reflection_info() const = 0;
};

/**
 * A program is a compiled shader program combined with reflection data.
 */
struct RHI_API Program : nvrhi::RefCounter<IProgram> {
    ProgramDesc get_desc() const override;
    nvrhi::ShaderDesc get_shader_desc() const override;
    void const* getBufferPointer() const override;
    size_t getBufferSize() const override;

    [[nodiscard]] const std::string& get_error_string() const override
    {
        return error_string;
    }
    const ShaderReflectionInfo& get_reflection_info() const override;

    template<typename T>
    void host_call(CPPPrelude::ComputeVaryingInput& input, T& uniform)
    {
        if (library) {
            auto func = reinterpret_cast<CPPPrelude::ComputeFunc>(
                library->findFuncByName(get_desc().entry_name.c_str()));
            if (func) {
                func(&input, NULL, &uniform);
            }
            else {
                throw std::runtime_error("Function not found.");
            }
        }
        else {
            throw std::runtime_error("Library not found.");
        }
    }

   private:
    friend class ShaderFactory;
    ShaderReflectionInfo reflection_info;
    Slang::ComPtr<ISlangBlob> blob;
    Slang::ComPtr<ISlangSharedLibrary> library;
    std::string error_string;
    ProgramDesc desc;
};

constexpr uint32_t c_FalcorMaterialInstanceSize = 128;

USTC_CG_NAMESPACE_CLOSE_SCOPE

USTC_CG_NAMESPACE_OPEN_SCOPE

#define DESC_HANDLE_TRAIT(RESOURCE)        \
    template<>                             \
    struct ResouceDesc<RESOURCE##Handle> { \
        using Desc = RESOURCE##Desc;       \
    };

#define HANDLE_DESC_TRAIT(RESOURCE)        \
    template<>                             \
    struct DescResouce<RESOURCE##Desc> {   \
        using Resource = RESOURCE##Handle; \
    };

template<typename RESOURCE>
struct ResouceDesc {
    using Desc = void;
};

template<typename DESC>
struct DescResouce {
    using Resource = void;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
