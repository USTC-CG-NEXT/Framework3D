
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "shaders/shaders/utils/HitObject.h"
#include "shaders/shaders/utils/ray.h"

#define WITH_NVAPI 1

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(scene_ray_launch)
{
    b.add_input<nvrhi::BufferHandle>("Pixel Target");
    b.add_input<nvrhi::BufferHandle>("Rays");

    b.add_output<nvrhi::BufferHandle>("Pixel Target");
    b.add_output<nvrhi::BufferHandle>("Hit Objects");
    b.add_output<int>("Buffer Size");
}

NODE_EXECUTION_FUNCTION(scene_ray_launch)
{
    Hd_USTC_CG_Camera* free_camera = get_free_camera(params);
    auto size = free_camera->dataWindow.GetSize();

    auto m_CommandList = resource_allocator.create(CommandListDesc{});

    auto rays = params.get_input<BufferHandle>("Rays");
    auto length = rays->getDesc().byteSize / sizeof(RayDesc);

    auto input_pixel_target_buffer =
        params.get_input<BufferHandle>("Pixel Target");

    BufferDesc hit_objects_desc;

    auto maximum_hit_object_count = size[0] * size[1];

    hit_objects_desc =
        BufferDesc{}
            .setByteSize((1 + maximum_hit_object_count) * sizeof(HitObjectInfo))
            .setCanHaveUAVs(true)
            .setInitialState(nvrhi::ResourceStates::CopyDest)
            .setKeepInitialState(true)
            .setStructStride(sizeof(HitObjectInfo));
    auto hit_objects = resource_allocator.create(hit_objects_desc);

    auto pixel_buffer_desc =
        BufferDesc{}
            .setByteSize(maximum_hit_object_count * sizeof(pxr::GfVec2i))
            .setStructStride(sizeof(pxr::GfVec2i))
            .setKeepInitialState(true)
            .setInitialState(nvrhi::ResourceStates::UnorderedAccess)
            .setCanHaveUAVs(true);
    auto pixel_target_buffer = resource_allocator.create(pixel_buffer_desc);

    // Read out the hit object buffer info.
    BufferDesc read_out_desc;
    read_out_desc.setByteSize(sizeof(HitObjectInfo))
        .setInitialState(nvrhi::ResourceStates::CopyDest)
        .setKeepInitialState(true)
        .setCpuAccess(nvrhi::CpuAccessMode::Read);
    auto read_out = resource_allocator.create(read_out_desc);
    MARK_DESTROY_NVRHI_RESOURCE(read_out);

    // 2. Prepare the shader

    ProgramDesc shader_compile_desc;
    shader_compile_desc.set_path(
        std::filesystem::path("shaders/ray_launch.slang"));
    shader_compile_desc.shaderType = nvrhi::ShaderType::AllRayTracing;

    auto raytrace_compiled = resource_allocator.create(shader_compile_desc);

    if (raytrace_compiled->get_error_string().empty()) {
        ShaderDesc shader_desc;
        shader_desc.entryName = "RayGen";
        shader_desc.shaderType = nvrhi::ShaderType::RayGeneration;
        shader_desc.debugName = std::to_string(
            reinterpret_cast<long long>(raytrace_compiled->getBufferPointer()));
        auto raygen_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());

        shader_desc.entryName = "ClosestHit";
        shader_desc.shaderType = nvrhi::ShaderType::ClosestHit;
        auto chs_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());

        shader_desc.entryName = "Miss";
        shader_desc.shaderType = nvrhi::ShaderType::Miss;
        auto miss_shader = resource_allocator.create(
            shader_desc,
            raytrace_compiled->getBufferPointer(),
            raytrace_compiled->getBufferSize());

        // 3. Prepare the hitgroup and pipeline

        nvrhi::BindingLayoutDesc globalBindingLayoutDesc;
        globalBindingLayoutDesc.visibility = nvrhi::ShaderType::All;
        globalBindingLayoutDesc.bindings = {
            { 0, nvrhi::ResourceType::RayTracingAccelStruct },
            { 1, nvrhi::ResourceType::StructuredBuffer_SRV },
            { 0, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 1, nvrhi::ResourceType::StructuredBuffer_UAV },
            { 2, nvrhi::ResourceType::StructuredBuffer_UAV }
        };
        auto globalBindingLayout =
            resource_allocator.create(globalBindingLayoutDesc);

        nvrhi::rt::PipelineDesc pipeline_desc;
        pipeline_desc.maxPayloadSize = 16 * sizeof(float);
        pipeline_desc.globalBindingLayouts = { globalBindingLayout };
        pipeline_desc.shaders = { { "RayGen", raygen_shader, nullptr },
                                  { "Miss", miss_shader, nullptr } };

        pipeline_desc.hitGroups = { {
            "HitGroup",
            chs_shader,
            nullptr,  // anyHitShader
            nullptr,  // intersectionShader
            nullptr,  // bindingLayout
            false     // isProceduralPrimitive
        } };
        auto m_TopLevelAS =
            params.get_global_payload<RenderGlobalPayload&>().TLAS;
        auto raytracing_pipeline = resource_allocator.create(pipeline_desc);

        BindingSetDesc binding_set_desc;
        binding_set_desc.bindings = nvrhi::BindingSetItemArray{
            nvrhi::BindingSetItem::RayTracingAccelStruct(0, m_TopLevelAS),
            nvrhi::BindingSetItem::StructuredBuffer_SRV(
                1, input_pixel_target_buffer.Get()),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(0, rays.Get()),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(1, hit_objects.Get()),
            nvrhi::BindingSetItem::StructuredBuffer_UAV(
                2, pixel_target_buffer.Get()),
        };
        auto binding_set = resource_allocator.create(
            binding_set_desc, globalBindingLayout.Get());

        nvrhi::rt::State state;
        nvrhi::rt::ShaderTableHandle sbt =
            raytracing_pipeline->createShaderTable();
        sbt->setRayGenerationShader("RayGen");
        sbt->addHitGroup("HitGroup");
        sbt->addMissShader("Miss");
        state.setShaderTable(sbt).addBindingSet(binding_set);

        m_CommandList->open();
        HitObjectInfo info;
        info.InstanceIndex = 0;
        m_CommandList->writeBuffer(
            hit_objects,
            &info,
            sizeof(HitObjectInfo),
            maximum_hit_object_count * sizeof(HitObjectInfo));

        m_CommandList->setRayTracingState(state);
        nvrhi::rt::DispatchRaysArguments args;
        args.width = length;
        m_CommandList->dispatchRays(args);

        m_CommandList->copyBuffer(
            read_out,
            0,
            hit_objects,
            maximum_hit_object_count * sizeof(HitObjectInfo),
            sizeof(HitObjectInfo));

        m_CommandList->close();
        resource_allocator.device->executeCommandList(m_CommandList);
        resource_allocator.device
            ->waitForIdle();  // This is not fully efficient.

        resource_allocator.destroy(raytracing_pipeline);
        resource_allocator.destroy(globalBindingLayout);
        resource_allocator.destroy(binding_set);
        resource_allocator.destroy(raygen_shader);
        resource_allocator.destroy(chs_shader);
        resource_allocator.destroy(miss_shader);
    }

    resource_allocator.destroy(m_CommandList);
    auto error = raytrace_compiled->get_error_string();
    resource_allocator.destroy(raytrace_compiled);

    params.set_output("Hit Objects", hit_objects);
    params.set_output("Pixel Target", pixel_target_buffer);

    auto cpu_read_out = resource_allocator.device->mapBuffer(
        read_out, nvrhi::CpuAccessMode::Read);
    HitObjectInfo info;
    memcpy(&info, cpu_read_out, sizeof(HitObjectInfo));
    resource_allocator.device->unmapBuffer(read_out);

    log::info("Buffer size: %s", +std::to_string(info.InstanceIndex).c_str());
    params.set_output("Buffer Size", static_cast<int>(info.InstanceIndex));
    if (error.size()) {
        log::warning(error.c_str());
        return false;
    }
    return true;
}

NODE_DECLARATION_UI(scene_ray_launch);
NODE_DEF_CLOSE_SCOPE
