
#include "RHI/internal/resources.hpp"
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "render_node_base.h"
#include "shaders/shaders/utils/CameraParameters.h"
#include "shaders/shaders/utils/ray.slang"
#include "shaders/shaders/utils/view_cb.h"
#include "utils/cam_to_view_contants.h"
#include "utils/math.h"
#include "utils/resource_cleaner.hpp"

NODE_DEF_OPEN_SCOPE

struct OldConstants {
    constexpr static bool has_storage = false;

    float aperture = 0;
    float focus_distance = 2;

    bool operator==(const OldConstants& rhs) const
    {
        return aperture == rhs.aperture && focus_distance == rhs.focus_distance;
    }

    bool operator!=(const OldConstants& rhs) const
    {
        return !(*this == rhs);
    }
};

NODE_DECLARATION_FUNCTION(node_render_ray_generation)
{
    b.add_input<nvrhi::TextureHandle>("random seeds");
    b.add_input<float>("Aperture").min(0).max(1).default_val(0);
    b.add_input<float>("Focus Distance").min(0).max(20).default_val(2);
    b.add_input<bool>("Scatter Rays").default_val(false);

    b.add_output<nvrhi::BufferHandle>("Pixel Target");
    b.add_output<nvrhi::BufferHandle>("Rays");
}

NODE_EXECUTION_FUNCTION(node_render_ray_generation)
{
    Hd_USTC_CG_Camera* free_camera = get_free_camera(params);
    auto image_size = free_camera->dataWindow.GetSize();

    auto aperture = params.get_input<float>("Aperture");
    auto focus_distance = params.get_input<float>("Focus Distance");

    if (params.get_storage<OldConstants>() !=
        OldConstants{ aperture, focus_distance }) {
        params.set_storage(OldConstants{ aperture, focus_distance });
        global_payload.reset_accumulation = true;
    }

    auto ray_buffer = create_buffer<RayInfo>(
        params, image_size[0] * image_size[1], false, true);

    auto pixel_target_buffer = create_buffer<GfVec2i>(
        params, image_size[0] * image_size[1], false, true);

    // 2. Prepare the shader
    std::string error_string;
    ShaderReflectionInfo reflection_info;
    std::vector<ShaderMacro> macro_defines;
    if (params.get_input<bool>("Scatter Rays"))
        macro_defines.push_back(ShaderMacro{ "SCATTER_RAYS", "1" });
    else
        macro_defines.push_back(ShaderMacro{ "SCATTER_RAYS", "0" });

    auto compute_shader = shader_factory.compile_shader(
        "main",
        nvrhi::ShaderType::Compute,
        "shaders/raygen.slang",
        reflection_info,
        error_string,
        macro_defines);
    MARK_DESTROY_NVRHI_RESOURCE(compute_shader);
    nvrhi::BindingLayoutDescVector binding_layout_desc_vec =
        reflection_info.get_binding_layout_descs();

    if (!error_string.empty()) {
        resource_allocator.destroy(ray_buffer);
        log::warning(error_string.c_str());
        return false;
    }
    auto binding_layout = resource_allocator.create(binding_layout_desc_vec[0]);
    MARK_DESTROY_NVRHI_RESOURCE(binding_layout);

    auto camera_param_cb = resource_allocator.create(
        BufferDesc{ .byteSize = sizeof(CameraParameters),
                    .debugName = "cameraParamCB",
                    .isConstantBuffer = true,
                    .initialState = nvrhi::ResourceStates::ConstantBuffer,
                    .cpuAccess = nvrhi::CpuAccessMode::Write });
    MARK_DESTROY_NVRHI_RESOURCE(camera_param_cb);

    ComputePipelineDesc pipeline_desc;
    pipeline_desc.CS = compute_shader;
    pipeline_desc.bindingLayouts = { binding_layout };
    auto compute_pipeline = resource_allocator.create(pipeline_desc);
    MARK_DESTROY_NVRHI_RESOURCE(compute_pipeline);

    auto command_list = resource_allocator.create(CommandListDesc{});
    MARK_DESTROY_NVRHI_RESOURCE(command_list);

    auto random_seeds = params.get_input<nvrhi::TextureHandle>("random seeds");

    auto constant_buffer = get_free_camera_planarview_cb(params);

    MARK_DESTROY_NVRHI_RESOURCE(constant_buffer);

    BindingSetDesc binding_set_desc;
    binding_set_desc.bindings = {
        nvrhi::BindingSetItem::StructuredBuffer_UAV(0, ray_buffer),
        nvrhi::BindingSetItem::Texture_UAV(1, random_seeds),
        nvrhi::BindingSetItem::StructuredBuffer_UAV(2, pixel_target_buffer),
        nvrhi::BindingSetItem::ConstantBuffer(0, constant_buffer),
        nvrhi::BindingSetItem::ConstantBuffer(1, camera_param_cb),
    };
    auto binding_set =
        resource_allocator.create(binding_set_desc, binding_layout.Get());
    MARK_DESTROY_NVRHI_RESOURCE(binding_set);

    command_list->open();

    CameraParameters camera_params;
    camera_params.aperture = aperture;
    camera_params.focusDistance = focus_distance;

    command_list->writeBuffer(
        camera_param_cb.Get(), &camera_params, sizeof(CameraParameters));
    nvrhi::ComputeState compute_state;
    compute_state.pipeline = compute_pipeline;
    compute_state.addBindingSet(binding_set);
    command_list->setComputeState(compute_state);
    command_list->dispatch(
        div_ceil(image_size[0], 32), div_ceil(image_size[1], 32));
    command_list->close();

    resource_allocator.device->executeCommandList(command_list);
    params.set_output("Rays", ray_buffer);
    params.set_output("Pixel Target", pixel_target_buffer);
    return true;
}

NODE_DECLARATION_UI(node_render_ray_generation);
NODE_DEF_CLOSE_SCOPE
