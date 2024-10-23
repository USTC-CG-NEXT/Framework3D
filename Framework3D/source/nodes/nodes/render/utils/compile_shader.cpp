#include "compile_shader.h"

#include "../NODES_FILES_DIR.h"
#include "../resource_allocator_instance.hpp"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
ShaderHandle compile_shader(
    const std::string& entryName,
    nvrhi::ShaderType shader_type,
    std::filesystem::path shader_path,
    nvrhi::BindingLayoutDescVector& binding_layout_desc,
    std::string& error_string,
    const std::vector<ShaderMacro>& macro_defines,
    bool nvapi_support,
    bool absolute)
{
    ProgramDesc shader_compile_desc;

    if (!absolute) {
        shader_path =
            std::filesystem::path(RENDER_NODES_FILES_DIR) / shader_path;
    }
    shader_compile_desc.set_entry_name(entryName);
    shader_compile_desc.set_path(shader_path);
    for (auto&& macro_define : macro_defines) {
        shader_compile_desc.define(macro_define.name, macro_define.definition);
    }
    shader_compile_desc.shaderType = shader_type;
    shader_compile_desc.nvapi_support = nvapi_support;
    auto shader_compiled = resource_allocator.create(shader_compile_desc);

    if (!shader_compiled->get_error_string().empty()) {
        error_string = shader_compiled->get_error_string();
        resource_allocator.destroy(shader_compiled);
        return nullptr;
    }

    ShaderDesc desc;
    desc.shaderType = shader_compile_desc.shaderType;
    desc.entryName = entryName;
    desc.debugName = std::to_string(
        reinterpret_cast<long long>(shader_compiled->getBufferPointer()));

    binding_layout_desc = shader_compiled->get_binding_layout_descs();

    auto compute_shader = resource_allocator.create(
        desc,
        shader_compiled->getBufferPointer(),
        shader_compiled->getBufferSize());

    resource_allocator.destroy(shader_compiled);

    return compute_shader;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE