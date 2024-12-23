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

    print("patches", patches.shape)
    patches = patches.reshape(-1, 4, 2)
    texture = imageio.imread(texture_name)
    torch_texture = torch.tensor(texture, device="cuda") / 255.0
    print("texture", torch_texture.shape)
    patch_uv_center = (
        1.0 / 4.0 * (patches[:, 0] + patches[:, 1] + patches[:, 2] + patches[:, 3])
    )

    print("patch_uv_center", patch_uv_center.shape)

    sampled_color = sample_texture_bilinear(torch_texture, flip_u(patch_uv_center))
    if sampled_color.shape[1] == 4:
        sampled_color = sampled_color[:, :3]
    elif sampled_color.shape[1] == 1:
        sampled_color = sampled_color.repeat(1, 3)

    image = torch.zeros(
        (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
    )

    image[targets[:, 0].long(), targets[:, 1].long()] = sampled_color

    return image
