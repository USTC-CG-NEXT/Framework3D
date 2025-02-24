
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hgiGL/computeCmds.h"
#include "renderTLAS.h"
#include "render_node_base.h"
#include "renderer/graphics_context.hpp"
#include "renderer/program_vars.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(rasterize)
{
    b.add_output<nvrhi::TextureHandle>("Position");
    b.add_output<nvrhi::TextureHandle>("Depth");
    b.add_output<nvrhi::TextureHandle>("Texcoords");
    b.add_output<nvrhi::TextureHandle>("DiffuseColor");
    b.add_output<nvrhi::TextureHandle>("MetallicRoughness");
    b.add_output<nvrhi::TextureHandle>("Normal");
}

NODE_EXECUTION_FUNCTION(rasterize)
{
    std::vector<ShaderMacro> macros{ { "ENABLE_DIFFUSE_COLOR", "1" },
                                     { "ENABLE_METALLIC_ROUGHNESS", "1" },
                                     { "ENABLE_NORMAL", "1" },
                                     { "ENABLE_TEXCOORD", "1" } };

    ProgramDesc vs_program_desc;
    vs_program_desc.shaderType = nvrhi::ShaderType::Vertex;
    vs_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("vs_main")
        .define(macros);

    ProgramHandle vs_program = resource_allocator.create(vs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(vs_program);
    CHECK_PROGRAM_ERROR(vs_program);

    ProgramDesc ps_program_desc;
    ps_program_desc.shaderType = nvrhi::ShaderType::Pixel;
    ps_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("ps_main")
        .define(macros);
    ProgramHandle ps_program = resource_allocator.create(ps_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(ps_program);
    CHECK_PROGRAM_ERROR(ps_program);

    auto view_cb = get_free_camera_planarview_cb(params);
    MARK_DESTROY_NVRHI_RESOURCE(view_cb);

    auto output_position = create_default_render_target(params);
    auto output_depth = create_default_depth_stencil(params);
    auto output_texcoords =
        create_default_render_target(params, nvrhi::Format::RG8_UNORM);
    auto output_diffuse_color = create_default_render_target(params);
    auto output_metallic_roughness = create_default_render_target(params);
    auto output_normal = create_default_render_target(params);

    ProgramVars program_vars(resource_allocator, vs_program, ps_program);
    program_vars["viewConstant"] = view_cb;
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

    GraphicsContext context(resource_allocator, program_vars);
    context.set_render_target(0, output_position)
        .set_render_target(1, output_texcoords)
        .set_render_target(2, output_diffuse_color)
        .set_render_target(3, output_metallic_roughness)
        .set_render_target(4, output_normal)
        .set_depth_stencil_target(output_depth)
        .finish_setting_frame_buffer();

    context.set_viewport(get_size(params)).finish_setting_pso();

    instance_collection->draw_indirect_pool.compress();

    GraphicsRenderState state;
    state.indexBuffer = { instance_collection->index_pool.get_device_buffer(),
                          nvrhi::Format::R32_UINT,
                          0 };

    // find the named mesh
    context.begin();
    context.draw_indirect(
        state,
        program_vars,
        instance_collection->draw_indirect_pool.get_device_buffer(),
        instance_collection->draw_indirect_pool.count());

    context.finish();

    params.set_output("Position", output_position);
    params.set_output("Depth", output_depth);
    params.set_output("Texcoords", output_texcoords);
    params.set_output("DiffuseColor", output_diffuse_color);
    params.set_output("MetallicRoughness", output_metallic_roughness);
    params.set_output("Normal", output_normal);

    return true;
}

NODE_DECLARATION_UI(rasterize);
NODE_DEF_CLOSE_SCOPE
