import torch
import glints.shaderAB as shader


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
    light_position,
):
    patches, worldToUV, targets = context.intersect_mesh_with_rays(
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix.flatten(),
        view_to_clip_matrix.flatten(),
    )

    reshaped_patches = patches.reshape(-1, 4, 2)

    diag_1 = reshaped_patches[:, 2, :] - reshaped_patches[:, 0, :]
    diag_2 = reshaped_patches[:, 3, :] - reshaped_patches[:, 1, :]
    l_diag_1 = torch.norm(diag_1, dim=1)
    l_diag_2 = torch.norm(diag_2, dim=1)

    intersect_width = torch.max(torch.cat((l_diag_1, l_diag_2)))

    intersection_pairs = scratch_context.intersect_line_with_rays(
        lines, patches, intersect_width
    )

    contribution_accumulation = torch.zeros(
        (patches.shape[0],), dtype=torch.float32, device="cuda"
    )

    intersected_lines = lines[intersection_pairs[:, 0].long()]
    intersected_patches = patches[intersection_pairs[:, 1].long()]
    intersected_targets = targets[intersection_pairs[:, 1].long()]
    intersected_worldToUV = worldToUV[intersection_pairs[:, 1].long()]

    camera_position_torch = torch.tensor(camera_position_np, device="cuda")

    if not isinstance(light_position, torch.Tensor):
        light_position_torch = torch.tensor(light_position, device="cuda")
    else:
        light_position_torch = light_position

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

    if lines.shape[1] == 2:
        contribution = shader.ShadeLineElement(
            intersected_lines,
            intersected_patches,
            transformed_camera_position,
            transformed_light_position,
            glints_roughness,
            width,
        )[:, 0]
    else:
        contribution = shader.ShadeBSplineElements(
            intersected_lines,
            intersected_patches,
            transformed_camera_position,
            transformed_light_position,
            glints_roughness,
            width,
        )[:, 0]

    assert torch.isnan(contribution).sum() == 0

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

    contribution_accumulation_on_lines = torch.zeros(
        (lines.shape[0],), dtype=torch.float32, device="cuda"
    )

    contribution_accumulation_on_lines.scatter_add_(
        0,
        intersection_pairs[:, 0].long(),
        contribution,
    )

    low_contribution_mask = contribution_accumulation_on_lines < 0.02 * torch.mean(
        contribution_accumulation_on_lines
    )

    return image, low_contribution_mask


import torch
import imageio


# texture shape (H, W, 3)
# uv shape (N, 2)
# return shape (N, 3)
def sample_texture_nearest(texture, uv):
    uv = torch.clamp(uv, 0.0, 1.0)
    uv = uv * torch.tensor([texture.shape[1], texture.shape[0]], device="cuda")
    uv = uv.long()
    return texture[uv[:, 1], uv[:, 0]]


def sample_texture_bilinear(texture, uv):
    uv = torch.clamp(uv, 0.0, 1.0)
    uv = uv * torch.tensor([texture.shape[1], texture.shape[0]], device="cuda")
    uv = uv.long()
    u = uv[:, 0]
    v = uv[:, 1]
    u_frac = (uv[:, 0].float() - u.float()).unsqueeze(1)
    v_frac = (uv[:, 1].float() - v.float()).unsqueeze(1)
    u = torch.clamp(u, 0, texture.shape[1] - 2)
    v = torch.clamp(v, 0, texture.shape[0] - 2)
    u_next = u + 1
    v_next = v + 1
    return (
        texture[v, u] * (1 - u_frac) * (1 - v_frac)
        + texture[v, u_next] * u_frac * (1 - v_frac)
        + texture[v_next, u] * (1 - u_frac) * v_frac
        + texture[v_next, u_next] * u_frac * v_frac
    )


def flip_u(uv):
    return torch.cat([1.0 - uv[:, 0:1], uv[:, 1:2]], dim=1)


def flip_v(uv):
    return torch.cat([uv[:, 0:1], 1.0 - uv[:, 1:2]], dim=1)


def prepare_target(
    texture_name,
    context,
    vertices,
    indices,
    vertex_buffer_stride,
    resolution,
    world_to_view_matrix,
    view_to_clip_matrix,
):
    patches, worldToUV, targets = context.intersect_mesh_with_rays(
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix.flatten(),
        view_to_clip_matrix.flatten(),
    )

    patches = patches.reshape(-1, 4, 2)
    texture = imageio.imread(texture_name)
    torch_texture = torch.tensor(texture, device="cuda") / 255.0

    torch_texture = (
        0.2126 * torch_texture[:, :, 0]
        + 0.7152 * torch_texture[:, :, 1]
        + 0.0722 * torch_texture[:, :, 2]
    )
    torch_texture = torch_texture.unsqueeze(2).repeat(1, 1, 3)

    patch_uv_center = (
        1.0 / 4.0 * (patches[:, 0] + patches[:, 1] + patches[:, 2] + patches[:, 3])
    )

    sampled_color = sample_texture_bilinear(
        torch_texture, flip_v(flip_u(patch_uv_center))
    )
    if sampled_color.shape[1] == 4:
        sampled_color = sampled_color[:, :3]
    elif sampled_color.shape[1] == 1:
        sampled_color = sampled_color.repeat(1, 3)

    image = torch.zeros(
        (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
    )

    image[targets[:, 0].long(), targets[:, 1].long()] = sampled_color

    return image
