
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "shaders/shaders/utils/HitObject.h"
#include "utils/math.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(material_eval_sample_pdf)
{
    b.add_input<nvrhi::BufferHandle>("PixelTarget");

    b.add_input<nvrhi::BufferHandle>("HitInfo");

    b.add_output<nvrhi::BufferHandle>("PixelTarget");
    b.add_input<int>("Buffer Size").min(1).max(10).default_val(4);
    b.add_output<nvrhi::BufferHandle>("Eval");
    b.add_output<nvrhi::BufferHandle>("Sample");
    b.add_output<nvrhi::BufferHandle>("Weight");
    b.add_output<nvrhi::BufferHandle>("Pdf");
}

NODE_EXECUTION_FUNCTION(material_eval_sample_pdf)
{
    using namespace nvrhi;

    // 0. Get the 'HitObjectInfos'

    auto hit_info_buffer = params.get_input<BufferHandle>("HitInfo");
    auto in_pixel_target_buffer = params.get_input<BufferHandle>("PixelTarget");

    auto length = hit_info_buffer->getDesc().byteSize / sizeof(HitObjectInfo);

    length = std::max(length, 1ull);

    // The Eval, Pixel Target together should be the same size, and should
    // together be able to store the result of the material evaluation

    auto buffer_desc = BufferDesc{}
                           .setByteSize(length * sizeof(pxr::GfVec2i))
                           .setStructStride(sizeof(pxr::GfVec2i))
                           .setKeepInitialState(true)
                           .setInitialState(ResourceStates::UnorderedAccess)
                           .setCanHaveUAVs(true);
    auto pixel_target_buffer = resource_allocator.create(buffer_desc);

    buffer_desc.setByteSize(length * sizeof(pxr::GfVec4f))
        .setStructStride(sizeof(pxr::GfVec4f));
    auto eval_buffer = resource_allocator.create(buffer_desc);

    buffer_desc.setByteSize(length * sizeof(RayInfo))
        .setStructStride(sizeof(RayInfo));
    auto sample_buffer = resource_allocator.create(buffer_desc);

    buffer_desc.setByteSize(length * sizeof(float))
        .setStructStride(sizeof(float));
    auto weight_buffer = resource_allocator.create(buffer_desc);

    // 'Pdf Should be just like float...'
    buffer_desc.setByteSize(length * sizeof(float))
        .setStructStride(sizeof(float));
    auto pdf_buffer = resource_allocator.create(buffer_desc);

    // Build the raytracing pipeline.

    // 2. Prepare the shader

    auto m_CommandList = resource_allocator.create(CommandListDesc{});
    MARK_DESTROY_NVRHI_RESOURCE(m_CommandList);

    ProgramDesc program_desc;
    program_desc.set_path(

        std::filesystem::path("shaders/material_eval_sample_pdf.slang"));
    program_desc.shaderType = nvrhi::ShaderType::AllRayTracing;
    program_desc.nvapi_support = true;
    program_desc.define(
        "FALCOR_MATERIAL_INSTANCE_SIZE",
        std::to_string(c_FalcorMaterialInstanceSize));

    auto raytrace_compiled = resource_allocator.create(program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(raytrace_compiled);

    auto buffer_size = params.get_input<int>("Buffer Size");

    if (raytrace_compiled->get_error_string().empty()) {
        ShaderDesc shader_desc;
        shader_desc.entryName = "RayGen";
        shader_desc.shaderType = nvrhi::ShaderType::RayGeneration;
        shader_desc.debugName = std::to_string(
            reinterpret_cast<long long>(raytrace_compiled->getBufferPointer()));
        shader_desc.hlslExtensionsUAV = 127;
        auto raygen_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());
        MARK_DESTROY_NVRHI_RESOURCE(raygen_shader);

        shader_desc.entryName = "ClosestHit";
        shader_desc.shaderType = nvrhi::ShaderType::ClosestHit;
        auto chs_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());
        MARK_DESTROY_NVRHI_RESOURCE(chs_shader);

        shader_desc.entryName = "Miss";
        shader_desc.shaderType = nvrhi::ShaderType::Miss;
        auto miss_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());
        MARK_DESTROY_NVRHI_RESOURCE(miss_shader);

        // 3. Prepare the hitgroup and pipeline

        nvrhi::BindingLayoutDesc globalBindingLayoutDesc;
        globalBindingLayoutDesc.visibility = nvrhi::ShaderType::All;
        globalBindingLayoutDesc.bindings = {
            { 0, nvrhi::ResourceType::RayTracingAccelStruct },
            { 1, nvrhi::ResourceType::StructuredBuffer_SRV },
            { 2, nvrhi::ResourceType::StructuredBuffer_SRV },
            { 0, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 1, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 2, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 3, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 4, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 127, nvrhi::ResourceType::TypedBuffer_UAV },
        };
        auto globalBindingLayout =
            resource_allocator.create(globalBindingLayoutDesc);
        MARK_DESTROY_NVRHI_RESOURCE(globalBindingLayout);

        nvrhi::rt::PipelineDesc pipeline_desc;
        pipeline_desc.maxPayloadSize = 16 * sizeof(float);
        pipeline_desc.globalBindingLayouts = { globalBindingLayout };
        pipeline_desc.shaders = { { "", raygen_shader, nullptr },
                                  { "", miss_shader, nullptr } };

        pipeline_desc.hitGroups = { {
            "HitGroup",
            chs_shader,
            nullptr,  // anyHitShader
            nullptr,  // intersectionShader
            nullptr,  // bindingLayout
            false     // isProceduralPrimitive
        } };

        pipeline_desc.setHlslExtensionsUAV(127);

        auto m_TopLevelAS =
            params.get_global_payload<RenderGlobalPayload&>().TLAS;
        auto raytracing_pipeline = resource_allocator.create(pipeline_desc);
        MARK_DESTROY_NVRHI_RESOURCE(raytracing_pipeline);

        BindingSetDesc binding_set_desc;
        binding_set_desc.bindings = nvrhi::BindingSetItemArray{
            nvrhi::BindingSetItem::RayTracingAccelStruct(0, m_TopLevelAS),
            nvrhi::BindingSetItem::StructuredBuffer_SRV(
                1, hit_info_buffer.Get()),
            nvrhi::BindingSetItem::StructuredBuffer_SRV(
                2, in_pixel_target_buffer.Get()),

            nvrhi::BindingSetItem::StructuredBuffer_UAV(0, pixel_target_buffer),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(1, eval_buffer),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(2, sample_buffer),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(3, weight_buffer),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(4, pdf_buffer),
            nvrhi::BindingSetItem::TypedBuffer_UAV(127, nullptr)
        };
        auto binding_set = resource_allocator.create(
            binding_set_desc, globalBindingLayout.Get());
        MARK_DESTROY_NVRHI_RESOURCE(binding_set);

        nvrhi::rt::State state;
        nvrhi::rt::ShaderTableHandle sbt =
            raytracing_pipeline->createShaderTable();
        sbt->setRayGenerationShader("RayGen");
        sbt->addHitGroup("HitGroup");
        sbt->addMissShader("Miss");
        state.setShaderTable(sbt).addBindingSet(binding_set);
        if (buffer_size > 0) {
            m_CommandList->open();
            m_CommandList->beginTrackingBufferState(
                hit_info_buffer, nvrhi::ResourceStates::UnorderedAccess);
            m_CommandList->setRayTracingState(state);
            nvrhi::rt::DispatchRaysArguments args;
            args.width = buffer_size;
            args.height = 1;
            m_CommandList->dispatchRays(args);
            m_CommandList->close();
            resource_allocator.device->executeCommandList(m_CommandList);
        }
    }
    else {
        resource_allocator.destroy(pixel_target_buffer);
        resource_allocator.destroy(eval_buffer);
        resource_allocator.destroy(sample_buffer);
        resource_allocator.destroy(weight_buffer);
        resource_allocator.destroy(pdf_buffer);
        log::warning(raytrace_compiled->get_error_string().c_str());
        return false;
    }

    // 4. Get the result
    params.set_output("PixelTarget", pixel_target_buffer);
    params.set_output("Eval", eval_buffer);
    params.set_output("Sample", sample_buffer);
    params.set_output("Weight", weight_buffer);
    params.set_output("Pdf", pdf_buffer);
    return true;
}

NODE_DECLARATION_UI(material_eval_sample_pdf);
NODE_DEF_CLOSE_SCOPE
