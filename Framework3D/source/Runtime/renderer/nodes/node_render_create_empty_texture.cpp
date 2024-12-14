
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(create_empty_texture)
{
    b.add_output<nvrhi::TextureHandle>("Texture");
}

NODE_EXECUTION_FUNCTION(create_empty_texture)
{
    Hd_USTC_CG_Camera* free_camera = get_free_camera(params);
    auto size = free_camera->dataWindow.GetSize();

    // 0. Prepare the output texture
    nvrhi::TextureDesc output_desc;
    output_desc.width = size[0];
    output_desc.height = size[1];
    output_desc.format = nvrhi::Format::RGBA32_FLOAT;
    output_desc.initialState = nvrhi::ResourceStates::UnorderedAccess;
    output_desc.keepInitialState = true;
    output_desc.isUAV = true;
    output_desc.clearValue = nvrhi::Color{ 0, 0, 0, 1 };
    auto output = resource_allocator.create(output_desc);

    auto command_list = resource_allocator.create(CommandListDesc{});
    MARK_DESTROY_NVRHI_RESOURCE(command_list);

    command_list->open();
    command_list->clearTextureFloat(output, {}, nvrhi::Color{ 0, 0, 0, 1 });
    command_list->close();

    resource_allocator.device->executeCommandList(command_list);

    params.set_output("Texture", output);
    return true;
}

NODE_DECLARATION_UI(create_empty_texture);
NODE_DEF_CLOSE_SCOPE
