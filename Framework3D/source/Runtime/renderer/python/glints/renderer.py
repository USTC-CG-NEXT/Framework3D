import torch
import glints.renderer as renderer
import glints.shader as shader

def render(
    context,
    scratch_context,
    lines,
    width,
    glints_roughness,
    vertices,
    indices,
    vertex_buffer_stride,
    resolution,
    world_to_view_matrix,
    view_to_clip_matrix,
    camera_position_np,
    light_position_np,
):
    patches, worldToUV, targets = context.intersect_mesh_with_rays(
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix.flatten(),
        view_to_clip_matrix.flatten(),
    )

    intersection_pairs = scratch_context.intersect_line_with_rays(
        lines, patches, float(width) * 3.0
    )
    print("intersection_pairs count", intersection_pairs.shape[0])

    contribution_accumulation = torch.zeros(
        (patches.shape[0],), dtype=torch.float32, device="cuda"
    )

    intersected_lines = lines[intersection_pairs[:, 0].long()]
    intersected_patches = patches[intersection_pairs[:, 1].long()]
    intersected_targets = targets[intersection_pairs[:, 1].long()]
    intersected_worldToUV = worldToUV[intersection_pairs[:, 1].long()]

    camera_position_torch = torch.tensor(camera_position_np, device="cuda")
    light_position_torch = torch.tensor(light_position_np, device="cuda")

    camera_position_homogeneous = torch.cat(
        [camera_position_torch, torch.ones(1, device="cuda", dtype=torch.float32)]
    )
    transformed_camera_position = torch.matmul(
        intersected_worldToUV, camera_position_homogeneous
    )
    transformed_camera_position = transformed_camera_position[
        :, :3
    ] / transformed_camera_position[:, 3].unsqueeze(1)

    light_position_homogeneous = torch.cat(
        [light_position_torch, torch.ones(1, device="cuda", dtype=torch.float32)]
    )
    transformed_light_position = torch.matmul(
        intersected_worldToUV, light_position_homogeneous
    )
    transformed_light_position = transformed_light_position[
        :, :3
    ] / transformed_light_position[:, 3].unsqueeze(1)

    intersected_patches = intersected_patches.reshape(-1, 4, 2)
    intersected_lines = intersected_lines[:, :, :2]

    contribution = shader.ShadeLineElement(
        intersected_lines,
        intersected_patches,
        transformed_camera_position,
        transformed_light_position,
        glints_roughness,
        width,
    )[:, 0]

    contribution = torch.nan_to_num(contribution, nan=0.0)

    contribution_accumulation.scatter_add_(
        0,
        intersection_pairs[:, 1].long(),
        contribution,
    )

    image = torch.zeros(
        (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
    )
    image[targets[:, 0].long(), targets[:, 1].long()] = (
        contribution_accumulation.unsqueeze(1).expand(-1, 3)
    )

    return image
