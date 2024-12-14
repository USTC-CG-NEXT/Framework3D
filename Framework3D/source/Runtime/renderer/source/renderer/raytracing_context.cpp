#include "raytracing_context.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
RaytracingContext::RaytracingContext(ResourceAllocator& r, ProgramVars& vars)
    : GPUContext(r, vars)
{
    program = vars.get_programs()[0];
}

RaytracingContext::~RaytracingContext()
{
    resource_allocator_.destroy(ray_generation_shader);
    for (auto& hitgroup : hit_groups) {
        resource_allocator_.destroy(std::get<0>(hitgroup));
        resource_allocator_.destroy(std::get<1>(hitgroup));
        resource_allocator_.destroy(std::get<2>(hitgroup));
    }
    for (auto& miss : miss_shaders) {
        resource_allocator_.destroy(miss);
    }
    resource_allocator_.destroy(raytracing_pipeline);
    resource_allocator_.destroy(sbt);
    resource_allocator_.destroy(program);
}

void RaytracingContext::begin()
{
    GPUContext::begin();
}

void RaytracingContext::finish()
{
    GPUContext::finish();
}

void RaytracingContext::trace_rays(
    const RaytracingState& state,
    const ProgramVars& program_vars,
    uint32_t width,
    uint32_t height,
    uint32_t depth) const
{
    nvrhi::rt::State rt_state;
    rt_state.bindings = program_vars.get_binding_sets();
    rt_state.shaderTable = sbt;

    commandList_->setRayTracingState(rt_state);

    nvrhi::rt::DispatchRaysArguments args;
    args.width = width;
    args.height = height;
    args.depth = depth;
    commandList_->dispatchRays(args);
}

void RaytracingContext::announce_raygeneration(const std::string& name)
{
    raygeneration_name = name;
}

void RaytracingContext::announce_hitgroup(
    const std::string& closesthit,
    const std::string& anyhit,
    const std::string& intercestion,

    unsigned position)
{
    if (hitgroup_names.size() <= position) {
        hitgroup_names.resize(position + 1);
    }
    hitgroup_names[position] =
        std::make_tuple(closesthit, anyhit, intercestion);
}

void RaytracingContext::announce_miss(
    const std::string& name,
    unsigned position)
{
    if (miss_names.size() <= position) {
        miss_names.resize(position + 1);
    }
    miss_names[position] = name;
}

void RaytracingContext::finish_announcing_shader_names()
{
    // prepare the shaders
    resource_allocator_.destroy(ray_generation_shader);
    for (auto& hitgroup : hit_groups) {
        resource_allocator_.destroy(std::get<0>(hitgroup));
        resource_allocator_.destroy(std::get<1>(hitgroup));
        resource_allocator_.destroy(std::get<2>(hitgroup));
    }
    for (auto& miss : miss_shaders) {
        resource_allocator_.destroy(miss);
    }

    nvrhi::ShaderDesc raygen_shader_desc;
    raygen_shader_desc.entryName = raygeneration_name.c_str();
    raygen_shader_desc.shaderType = nvrhi::ShaderType::RayGeneration;
    raygen_shader_desc.debugName = std::to_string(
        reinterpret_cast<long long>(program->getBufferPointer()));
    ray_generation_shader = resource_allocator_.create(
        raygen_shader_desc,
        program->getBufferPointer(),
        program->getBufferSize());

    for (auto& hitgroup : hitgroup_names) {
        nvrhi::ShaderDesc chs_desc;
        chs_desc.entryName = std::get<0>(hitgroup).c_str();
        chs_desc.shaderType = nvrhi::ShaderType::ClosestHit;
        chs_desc.debugName = std::to_string(
            reinterpret_cast<long long>(program->getBufferPointer()));
        assert(!chs_desc.entryName.empty());
        auto chs_shader = resource_allocator_.create(
            chs_desc, program->getBufferPointer(), program->getBufferSize());

        nvrhi::ShaderDesc ahs_desc;
        ahs_desc.entryName = std::get<1>(hitgroup).c_str();
        ahs_desc.shaderType = nvrhi::ShaderType::AnyHit;
        ahs_desc.debugName = std::to_string(
            reinterpret_cast<long long>(program->getBufferPointer()));

        nvrhi::ShaderHandle ahs_shader = nullptr;
        if (!ahs_desc.entryName.empty()) {
            ahs_shader = resource_allocator_.create(
                ahs_desc,
                program->getBufferPointer(),
                program->getBufferSize());
        }

        nvrhi::ShaderDesc is_desc;
        is_desc.entryName = std::get<2>(hitgroup).c_str();
        is_desc.shaderType = nvrhi::ShaderType::Intersection;
        is_desc.debugName = std::to_string(
            reinterpret_cast<long long>(program->getBufferPointer()));

        nvrhi::ShaderHandle is_shader = nullptr;
        if (!is_desc.entryName.empty()) {
            is_shader = resource_allocator_.create(
                is_desc, program->getBufferPointer(), program->getBufferSize());
        }

        hit_groups.push_back(
            std::make_tuple(chs_shader, ahs_shader, is_shader));
    }

    for (auto& miss : miss_names) {
        nvrhi::ShaderDesc miss_desc;
        miss_desc.entryName = miss.c_str();
        miss_desc.shaderType = nvrhi::ShaderType::Miss;
        miss_desc.debugName = std::to_string(
            reinterpret_cast<long long>(program->getBufferPointer()));
        auto miss_shader = resource_allocator_.create(
            miss_desc, program->getBufferPointer(), program->getBufferSize());
        miss_shaders.push_back(miss_shader);
    }

    nvrhi::rt::PipelineDesc pipeline_desc;
    pipeline_desc.maxPayloadSize = 16 * sizeof(float);
    pipeline_desc.globalBindingLayouts = vars_.get_binding_layout();

    pipeline_desc.shaders = { { "Raygen", ray_generation_shader, nullptr } };

    for (size_t i = 0; i < hit_groups.size(); ++i) {
        std::string hit_group_export_name = "HitGroup" + std::to_string(i);
        pipeline_desc.hitGroups.push_back({ hit_group_export_name,
                                            std::get<0>(hit_groups[i]),
                                            std::get<1>(hit_groups[i]),
                                            std::get<2>(hit_groups[i]) });
    }

    for (size_t i = 0; i < miss_shaders.size(); ++i) {
        std::string miss_export_name = "Miss" + std::to_string(i);
        pipeline_desc.shaders.push_back({ miss_export_name, miss_shaders[i] });
    }

    resource_allocator_.destroy(raytracing_pipeline);
    raytracing_pipeline = resource_allocator_.create(pipeline_desc);

    sbt = raytracing_pipeline->createShaderTable();
    sbt->setRayGenerationShader("Raygen");
    for (size_t i = 0; i < hit_groups.size(); ++i) {
        std::string hit_group_export_name = "HitGroup" + std::to_string(i);
        sbt->addHitGroup(hit_group_export_name.c_str());
    }
    for (size_t i = 0; i < miss_shaders.size(); ++i) {
        std::string miss_export_name = "Miss" + std::to_string(i);
        sbt->addMissShader(miss_export_name.c_str());
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE