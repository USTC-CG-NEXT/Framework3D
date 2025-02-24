
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"

NODE_DEF_OPEN_SCOPE
struct Storage {
    constexpr static bool has_storage = false;

    int current_spp = 0;
    nvrhi::TextureHandle accumulated;

    PlanarViewConstants old_constants;
};

NODE_DECLARATION_FUNCTION(accumulate)
{
    // Function content omitted
    b.add_input<nvrhi::TextureHandle>("Texture");
    b.add_input<int>("Max Samples").min(0).max(64).default_val(16);

    b.add_output<nvrhi::TextureHandle>("Accumulated");
}

NODE_EXECUTION_FUNCTION(accumulate)
{
    ProgramDesc cs_program_desc;
    cs_program_desc.shaderType = nvrhi::ShaderType::Compute;
    cs_program_desc.set_path("shaders/accumulate.slang").set_entry_name("main");
    ProgramHandle cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto texture = params.get_input<nvrhi::TextureHandle>("Texture");
    auto max_samples = params.get_input<int>("Max Samples");

    auto& storage = params.get_storage<Storage&>();
    if (!storage.accumulated ||
        storage.accumulated->getDesc() != texture->getDesc()) {
        auto desc = texture->getDesc();

        storage.accumulated = resource_allocator.device->createTexture(desc);
        initialize_texture(
            params, storage.accumulated, nvrhi::Color{ 0, 0, 0, 1 });

        storage.current_spp = 0;
    }

    if (storage.old_constants !=
        camera_to_view_constants(get_free_camera(params))) {
        storage.current_spp = 0;
        storage.old_constants =
            camera_to_view_constants(get_free_camera(params));
    }

    if (global_payload.reset_accumulation) {
        storage.current_spp = 0;
    }

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["Texture"] = texture;
    program_vars["Accumulated"] = storage.accumulated;

    auto spp_cb = create_constant_buffer(params, storage.current_spp);
    MARK_DESTROY_NVRHI_RESOURCE(spp_cb);

    program_vars["CurrentSPP"] = spp_cb;

    auto image_size =
        GfVec2i(texture->getDesc().width, texture->getDesc().height);
    auto size_cb = create_constant_buffer(params, image_size);
    MARK_DESTROY_NVRHI_RESOURCE(size_cb);

    program_vars["ImageSize"] = size_cb;

    program_vars.finish_setting_vars();

    ComputeContext context(resource_allocator, program_vars);

    context.finish_setting_pso();

    context.begin();
    context.dispatch({}, program_vars, image_size[0], 32, image_size[1], 32);
    context.finish();

    storage.current_spp++;

    params.set_output("Accumulated", storage.accumulated);
}

NODE_DECLARATION_UI(accumulate);
NODE_DEF_CLOSE_SCOPE
