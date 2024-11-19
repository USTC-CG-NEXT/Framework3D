
#include <diff_optics/diff_optics.hpp>

#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"
#include "shaders/shaders/utils/ray.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(physical_lens_raygen)
{
    // Function content omitted

    b.add_input<TextureHandle>("Random Number");

    b.add_output<BufferHandle>("Rays");
    b.add_output<BufferHandle>("Pixel Target");
}

NODE_EXECUTION_FUNCTION(physical_lens_raygen)
{
    ProgramDesc cs_program_desc;
    cs_program_desc.shaderType = nvrhi::ShaderType::Compute;
    cs_program_desc.set_path("shaders/physical_lens_raygen.slang")
        .set_entry_name("main");
    ProgramHandle cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto random_number = params.get_input<TextureHandle>("Random Number");

    // Create the rays buffer

    auto image_size = get_size(params);

    auto ray_buffer =
        create_buffer<RayInfo>(params, image_size[0] * image_size[1]);
    auto pixel_target_buffer =
        create_buffer<GfVec2i>(params, image_size[0] * image_size[1]);

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["random_seeds"] = random_number;
    program_vars["rays"] = ray_buffer;


    auto size_cb = create_buffer<GfVec2i>()


    params.set_output("Rays", ray_buffer);

    return true;
}

NODE_DECLARATION_UI(physical_lens_raygen);
NODE_DEF_CLOSE_SCOPE
