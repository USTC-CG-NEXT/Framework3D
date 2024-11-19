
#include <diff_optics/diff_optics.hpp>
#include <fstream>

#include "diff_optics/lens_system_compiler.hpp"
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"
#include "shaders/shaders/utils/ray.h"

NODE_DEF_OPEN_SCOPE

struct PhysicalLensStorage {
    constexpr static bool has_storage = false;

    bool compiled = false;
    CompiledDataBlock compiled_block;
};

NODE_DECLARATION_FUNCTION(physical_lens_raygen)
{
    // Function content omitted

    b.add_input<TextureHandle>("Random Number");
    b.add_input<float>("Focus distance").min(25).max(75).default_val(35);

    b.add_output<BufferHandle>("Pixel Target");
    b.add_output<BufferHandle>("Rays");
}

void compile_lens_system(LensSystem* lens_system, ExeParams& params)
{
    LensSystemCompiler compiler;
    auto [shader, compiled_block] = compiler.compile(lens_system, false);

    // write shader (std::string) to lens_shader.slang

    auto file = std::ofstream("lens_shader.slang");
    file << shader;
    file.close();

    params.get_storage<PhysicalLensStorage&>().compiled = true;
    params.get_storage<PhysicalLensStorage&>().compiled_block = compiled_block;
}

NODE_EXECUTION_FUNCTION(physical_lens_raygen)
{
    if (params.get_storage<PhysicalLensStorage&>().compiled == false) {
        auto lens_system = global_payload.lens_system;
        compile_lens_system(lens_system, params);
    }

    ProgramDesc cs_program_desc;
    cs_program_desc.shaderType = nvrhi::ShaderType::Compute;
    cs_program_desc.set_path("shaders/physical_lens_raygen.slang")
        .set_entry_name("computeMain");
    ProgramHandle cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto random_number = params.get_input<TextureHandle>("Random Number");

    // Create the rays buffer

    auto image_size = get_size(params);

    auto ray_buffer = create_buffer<RayInfo>(
        params, image_size[0] * image_size[1], false, true);
    auto pixel_target_buffer = create_buffer<GfVec2i>(
        params, image_size[0] * image_size[1], false, true);

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["random_seeds"] = random_number;
    program_vars["rays"] = ray_buffer;
    program_vars["pixel_targets"] = pixel_target_buffer;

    auto size_cb = create_buffer(params, 1, image_size, true);
    MARK_DESTROY_NVRHI_RESOURCE(size_cb);
    program_vars["size"] = size_cb;

    auto& compiled_block =
        params.get_storage<PhysicalLensStorage&>().compiled_block;

    compiled_block.parameters[0] = 36;
    compiled_block.parameters[1] = (36.f * image_size[1]) / image_size[0];

    compiled_block.parameters[2] = *reinterpret_cast<float*>(&image_size[0]);
    compiled_block.parameters[3] = *reinterpret_cast<float*>(&image_size[1]);
    compiled_block.parameters[4] = params.get_input<float>("Focus distance");

    auto lens_cb =
        create_buffer<float>(params, compiled_block.parameters.size(), true);
    MARK_DESTROY_NVRHI_RESOURCE(lens_cb);

    LensSystemCompiler::fill_block_data(
        global_payload.lens_system, compiled_block);

    auto ptr = resource_allocator.device->mapBuffer(
        lens_cb, nvrhi::CpuAccessMode::Write);

    memcpy(
        ptr,
        compiled_block.parameters.data(),
        compiled_block.parameters.size() * sizeof(float));

    resource_allocator.device->unmapBuffer(lens_cb);
    program_vars["lens_system_data"] = lens_cb;

    auto view_cb = get_free_camera_planarview_cb(params);
    MARK_DESTROY_NVRHI_RESOURCE(view_cb);

    program_vars["viewConstant"] = view_cb;

    program_vars.finish_setting_vars();

    ComputeContext context(resource_allocator, program_vars);
    context.finish_setting_pso();

    context.begin();
    context.dispatch({}, program_vars, image_size[0], 32, image_size[1], 32);
    context.finish();

    params.set_output("Rays", ray_buffer);
    params.set_output("Pixel Target", pixel_target_buffer);
    return true;
}

NODE_DECLARATION_UI(physical_lens_raygen);
NODE_DEF_CLOSE_SCOPE
