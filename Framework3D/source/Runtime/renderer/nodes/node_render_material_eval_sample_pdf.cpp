
#include "../source/renderTLAS.h"
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "renderer/raytracing_context.hpp"
#include "shaders/shaders/utils/HitObject.h"
#include "utils/math.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(material_eval_sample_pdf)
{
    b.add_input<nvrhi::BufferHandle>("PixelTarget");
    b.add_input<nvrhi::BufferHandle>("HitInfo");
    b.add_input<nvrhi::BufferHandle>("Random Seeds");

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
    CHECK_PROGRAM_ERROR(raytrace_compiled);

    auto m_CommandList = resource_allocator.create(CommandListDesc{});
    MARK_DESTROY_NVRHI_RESOURCE(m_CommandList);

    // 0. Get the 'HitObjectInfos'

    auto hit_info_buffer = params.get_input<BufferHandle>("HitInfo");
    auto in_pixel_target_buffer = params.get_input<BufferHandle>("PixelTarget");

    auto length = hit_info_buffer->getDesc().byteSize / sizeof(HitObjectInfo);

    length = std::max(length, static_cast<decltype(length)>(1));

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

    auto random_seeds = params.get_input<BufferHandle>("Random Seeds");
    // Set the program variables

    ProgramVars program_vars(resource_allocator, raytrace_compiled);
    program_vars["SceneBVH"] = params.get_global_payload<RenderGlobalPayload&>()
                                   .InstanceCollection->get_tlas();
    program_vars["hitObjects"] = hit_info_buffer;
    program_vars["in_PixelTarget"] = in_pixel_target_buffer;
    program_vars["PixelTarget"] = pixel_target_buffer;
    program_vars["Eval"] = eval_buffer;
    program_vars["Sample"] = sample_buffer;
    program_vars["Weight"] = weight_buffer;
    program_vars["Pdf"] = pdf_buffer;
    program_vars["random_seeds"] = random_seeds;
    program_vars["index_buffer"] =
        instance_collection->index_pool.get_device_buffer();

    program_vars["instanceDescBuffer"] =
        instance_collection->instance_pool.get_device_buffer();
    program_vars["meshDescBuffer"] =
        instance_collection->mesh_pool.get_device_buffer();

    DescriptorHandle handle =
        instance_collection->bindlessData.descriptorTableManager
            ->CreateDescriptorHandle(
                nvrhi::BindingSetItem::StructuredBuffer_SRV(
                    0, instance_collection->vertex_pool.get_device_buffer()));

    program_vars.set_descriptor_table(
        "t_BindlessBuffers",
        instance_collection->bindlessData.descriptorTableManager
            ->GetDescriptorTable(),
        instance_collection->bindlessData.bindlessLayout);

    program_vars.finish_setting_vars();

    RaytracingContext context(resource_allocator, program_vars);

    context.announce_raygeneration("RayGen");
    context.announce_hitgroup("ClosestHit");
    context.announce_hitgroup("ShadowHit", "", "", 1);
    context.announce_miss("Miss");
    context.announce_miss("ShadowMiss", 1);
    context.finish_announcing_shader_names();

    // 2. Prepare the shader

    auto buffer_size = params.get_input<int>("Buffer Size");

    if (buffer_size > 0) {
        context.begin();
        context.trace_rays({}, program_vars, buffer_size, 1, 1);
        context.finish();
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
