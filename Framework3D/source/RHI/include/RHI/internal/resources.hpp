#pragma once

#include <wrl.h>

#include <filesystem>

#include "rhi/api.h"
#include "map.h"
#include "nvrhi/nvrhi.h"
#include "slang-com-ptr.h"

namespace nvrhi {
using CommandListDesc = nvrhi::CommandListParameters;
typedef static_vector<BindingLayoutDesc, c_MaxBindingLayouts>
    BindingLayoutDescVector;

struct StagingTextureDesc : public nvrhi::TextureDesc { };

struct CPUBuffer {
    void* data;

    ~CPUBuffer()
    {
        delete[] data;
    }
};

struct CPUBufferDesc {
    size_t size;
};

using CPUBufferHandle = std::shared_ptr<CPUBuffer>;
}  // namespace nvrhi

#include "nvrhi_equality.hpp"

struct IDxcBlob;
USTC_CG_NAMESPACE_OPEN_SCOPE
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

using ProgramHandle = nvrhi::RefCountPtr<Program>;

class IProgram : public nvrhi::IResource {
   public:
    virtual void const* getBufferPointer() const = 0;
    virtual size_t getBufferSize() const = 0;
    virtual [[nodiscard]] const std::string& get_error_string() const = 0;
    virtual [[nodiscard]] const nvrhi::BindingLayoutDescVector&
    get_binding_layout_descs() const = 0;
};

struct Program : nvrhi::RefCounter<IProgram> {
    void const* getBufferPointer() const override;
    size_t getBufferSize() const override;

    [[nodiscard]] const std::string& get_error_string() const override
    {
        return error_string;
    }

    [[nodiscard]] const nvrhi::BindingLayoutDescVector&
    get_binding_layout_descs() const override
    {
        return binding_layout_;
    }

   private:
    friend class ShaderFactory;

    nvrhi::BindingLayoutDescVector binding_layout_;
    Slang::ComPtr<ISlangBlob> blob;
    std::string error_string;
};

struct ShaderMacro {
    std::string name;
    std::string definition;

    ShaderMacro(const std::string& _name, const std::string& _definition)
        : name(_name),
          definition(_definition)
    {
    }
};

struct ProgramDesc {
    friend bool operator==(const ProgramDesc& lhs, const ProgramDesc& rhs)
    {
        return lhs.path == rhs.path && lhs.entry_name == rhs.entry_name &&
               lhs.lastWriteTime == rhs.lastWriteTime &&
               lhs.shaderType == rhs.shaderType &&
               lhs.nvapi_support == rhs.nvapi_support;
    }

    friend bool operator!=(const ProgramDesc& lhs, const ProgramDesc& rhs)
    {
        return !(lhs == rhs);
    }

    void define(std::string macro, std::string value)
    {
        macros.push_back(ShaderMacro(macro, value));
    }
    void set_path(const std::filesystem::path& path);
    void set_shader_type(nvrhi::ShaderType shaderType);
    void set_entry_name(const std::string& entry_name);

    nvrhi::ShaderType shaderType;
    bool nvapi_support = false;

   private:
    void update_last_write_time(const std::filesystem::path& path);
    std::vector<ShaderMacro> macros;
    std::string get_profile() const;
    friend class ShaderFactory;
    std::filesystem::path path;
    std::string source_code;
    std::filesystem::file_time_type lastWriteTime;
    std::string entry_name;
};

ProgramHandle createProgram(const ProgramDesc& desc);

// Function to merge two BindingLayoutDescVector objects
nvrhi::BindingLayoutDescVector mergeBindingLayoutDescVectors(
    const nvrhi::BindingLayoutDescVector& vec1,
    const nvrhi::BindingLayoutDescVector& vec2);

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
