
#include "RHI/internal/nvrhi_patch.hpp"
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "render_node_base.h"
#include "shaders/shaders/utils/cpp_shader_macro.h"
#include "utils/math.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(render_scatter_contribution)
{
    b.add_input<nvrhi::BufferHandle>("PixelTarget");
    b.add_input<nvrhi::BufferHandle>("Eval");
    b.add_input<int>("Buffer Size");
    b.add_input<nvrhi::TextureHandle>("Source Texture");

    b.add_output<nvrhi::TextureHandle>("Result Texture");
}

NODE_EXECUTION_FUNCTION(render_scatter_contribution)
{
    using namespace nvrhi;

    auto pixel_target_buffer = params.get_input<BufferHandle>("PixelTarget");
    auto eval_buffer = params.get_input<BufferHandle>("Eval");
    auto source_texture = params.get_input<TextureHandle>("Source Texture");
    auto length = params.get_input<int>("Buffer Size");
    if (length > 0) {
        std::string error_string;
        nvrhi::BindingLayoutDescVector binding_layout_desc;
        auto compute_shader = shader_factory.compile_shader(
            "main",
            ShaderType::Compute,
            "shaders/scatter.slang",
            binding_layout_desc,
            error_string,
            {},
            {},
            false);
        MARK_DESTROY_NVRHI_RESOURCE(compute_shader);

        // Constant buffer contains the size of the length (single float), and I
        // can write if from CPU
        auto cb_desc = BufferDesc{}
                           .setByteSize(sizeof(float))
                           .setInitialState(ResourceStates::CopyDest)
                           .setKeepInitialState(true)
                           .setCpuAccess(CpuAccessMode::Write)
                           .setIsConstantBuffer(true);

        auto cb = resource_allocator.create(cb_desc);
        MARK_DESTROY_NVRHI_RESOURCE(cb);

        auto binding_layout = resource_allocator.create(binding_layout_desc[0]);
        MARK_DESTROY_NVRHI_RESOURCE(binding_layout);

        // BindingSet and BindingSetLayout
        BindingSetDesc binding_set_desc;
        binding_set_desc.bindings = {
            nvrhi::BindingSetItem::StructuredBuffer_SRV(0, eval_buffer),
            nvrhi::BindingSetItem::StructuredBuffer_SRV(1, pixel_target_buffer),
            nvrhi::BindingSetItem::ConstantBuffer(0, cb),
            nvrhi::BindingSetItem::Texture_UAV(0, source_texture)
        };

        auto binding_set =
            resource_allocator.create(binding_set_desc, binding_layout.Get());
        MARK_DESTROY_NVRHI_RESOURCE(binding_set);
        if (!binding_set) {
            // Handle error
            return false;
        }
        // Execute the shader
        ComputePipelineDesc pipeline_desc;
        pipeline_desc.CS = compute_shader;
        pipeline_desc.bindingLayouts = { binding_layout };
        auto pipeline = resource_allocator.create(pipeline_desc);

        MARK_DESTROY_NVRHI_RESOURCE(pipeline);
        if (!pipeline) {
            // Handle error
            return false;
        }

        ComputeState compute_state;
        compute_state.pipeline = pipeline;
        compute_state.bindings = { binding_set };

        CommandListHandle command_list =
            resource_allocator.create(CommandListDesc{});
        MARK_DESTROY_NVRHI_RESOURCE(command_list);

        command_list->open();
        command_list->writeBuffer(cb, &length, sizeof(length));
        command_list->setComputeState(compute_state);
        command_list->dispatch(div_ceil(length, 64), 1, 1);
        command_list->close();
        resource_allocator.device->executeCommandList(command_list);
    }
    params.set_output("Result Texture", source_texture);
    return true;
}

NODE_DECLARATION_UI(render_scatter_contribution);
NODE_DEF_CLOSE_SCOPE
