#include "nodes/core/def/node_def.hpp"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "utils/math.h"
NODE_DEF_OPEN_SCOPE
struct RNGStorage {
    static constexpr bool has_storage = false;
    nvrhi::TextureHandle random_number = nullptr;
};

// This texture is for repeated read and write.
NODE_DECLARATION_FUNCTION(rng_texture)
{
    b.add_output<nvrhi::TextureHandle>("Random Number");
}

NODE_EXECUTION_FUNCTION(rng_texture)
{
    Hd_USTC_CG_Camera* free_camera = get_free_camera(params);
    auto size = free_camera->dataWindow.GetSize();

    nvrhi::TextureDesc output_desc;
    output_desc.debugName = "Random Number Texture";
    output_desc.width = size[0];
    output_desc.height = size[1];
    output_desc.format = nvrhi::Format::R32_UINT;
    output_desc.initialState = nvrhi::ResourceStates::UnorderedAccess;
    output_desc.keepInitialState = true;
    output_desc.isUAV = true;

    auto& stored_rng = params.get_storage<RNGStorage&>();
    bool first_attempt = stored_rng.random_number == nullptr ||
                         output_desc != stored_rng.random_number->getDesc();
    if (first_attempt) {
        resource_allocator.destroy(stored_rng.random_number);
        stored_rng.random_number = resource_allocator.create(output_desc);
    }

    std::string error_string;

    ShaderHandle compute_shader;
    ShaderReflectionInfo reflection;
    if (first_attempt) {
        compute_shader = shader_factory.compile_shader(
            "main",
            nvrhi::ShaderType::Compute,
            "shaders/utils/random_init.slang",
            reflection,
            error_string);
    }
    else {
        compute_shader = shader_factory.compile_shader(
            "main",
            nvrhi::ShaderType::Compute,
            "shaders/utils/random_step.slang",
            reflection,
            error_string);
    }
    nvrhi::BindingLayoutDescVector binding_layout_descs =
        reflection.get_binding_layout_descs();
    if (!compute_shader) {
        log::warning(error_string.c_str());
        return false;
    }

    nvrhi::BindingLayoutVector binding_layouts;
    binding_layouts.resize(binding_layout_descs.size());

    for (int i = 0; i < binding_layout_descs.size(); ++i) {
        binding_layouts[i] = resource_allocator.create(binding_layout_descs[i]);
    }

    nvrhi::ComputePipelineDesc pipeline_desc;
    pipeline_desc.CS = compute_shader;
    pipeline_desc.bindingLayouts = binding_layouts;

    auto compute_pipeline = resource_allocator.create(pipeline_desc);

    auto command_list = resource_allocator.create(CommandListDesc{});

    BindingSetDesc binding_set_desc = { { nvrhi::BindingSetItem::Texture_UAV(
        0, stored_rng.random_number) } };

    auto binding_set_0 =
        resource_allocator.create(binding_set_desc, binding_layouts[0]);

    command_list->open();

    nvrhi::ComputeState compute_state;
    compute_state.pipeline = compute_pipeline;
    compute_state.bindings = { binding_set_0 };

    command_list->setComputeState(compute_state);

    auto texture_info = stored_rng.random_number->getDesc();
    command_list->dispatch(
        div_ceil(texture_info.width, 16), div_ceil(texture_info.height, 16));
    command_list->close();
    resource_allocator.device->executeCommandList(command_list);

    resource_allocator.destroy(compute_shader);
    for (int i = 0; i < binding_layouts.size(); ++i) {
        resource_allocator.destroy(binding_layouts[0]);
    }
    resource_allocator.destroy(binding_set_0);
    resource_allocator.destroy(compute_pipeline);
    resource_allocator.destroy(command_list);

    params.set_output("Random Number", stored_rng.random_number);
    return true;
}

NODE_DECLARATION_UI(rng_texture);
NODE_DEF_CLOSE_SCOPE
