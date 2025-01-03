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
    force_single_line=False,
    line_weight=None,
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
    if not force_single_line:
        intersect_width = torch.max(torch.cat((l_diag_1, l_diag_2)))
    else:
        intersect_width = torch.min(torch.cat((l_diag_1, l_diag_2))) / 1000

    intersection_pairs = scratch_context.intersect_line_with_rays(
        lines, patches, intersect_width
    )

    contribution_accumulation = torch.zeros(
        (patches.shape[0],), dtype=torch.float32, device="cuda"
    )

    intersected_lines = lines[intersection_pairs[:, 0].long()]
    if line_weight is not None:
        intersect_lines_weight = line_weight[intersection_pairs[:, 0].long()]
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

    if line_weight is not None:
        contribution = contribution * intersect_lines_weight

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

    low_contribution_mask = contribution_accumulation_on_lines < 0.001 * torch.mean(
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
    uv = uv * torch.tensor([texture.shape[0], texture.shape[1]], device="cuda")
    uv = uv.long()
    return texture[uv[:, 0], uv[:, 1]]


def sample_texture_bilinear(texture, uv):
    uv = torch.clamp(uv, 0.0, 1.0)
    uv = uv * torch.tensor([texture.shape[0], texture.shape[1]], device="cuda")
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
        texture[u, v] * (1 - u_frac) * (1 - v_frac)
        + texture[u_next, v] * u_frac * (1 - v_frac)
        + texture[u, v_next] * (1 - u_frac) * v_frac
        + texture[u_next, v_next] * u_frac * v_frac
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
    texture = test_utils.read_image(texture_name)
    torch_texture = torch.tensor(texture, device="cuda")

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


import glints.test_utils as test_utils


def target_bake_to_texture(
    target_name,
    context,
    vertices,
    indices,
    vertex_buffer_stride,
    uv_resolution,
    world_to_view_matrix,
    view_to_clip_matrix,
):
    """
    The inverse process of prepare_target. This function bakes the target image to a texture.
    """

    if isinstance(target_name, str):
        target_image = test_utils.read_image(target_name)
    else:
        target_image = target_name
    resolution = target_image.shape[:2]

    patches, worldToUV, targets = context.intersect_mesh_with_rays(
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix.flatten(),
        view_to_clip_matrix.flatten(),
    )

    patches = patches.reshape(-1, 4, 2)
    patch_uv_center = (
        1.0 / 4.0 * (patches[:, 0] + patches[:, 1] + patches[:, 2] + patches[:, 3])
    )
    assert torch.all(patch_uv_center >= 0) and torch.all(patch_uv_center <= 1)
    uv_center_pixel_id = patch_uv_center * torch.tensor(uv_resolution, device="cuda")

    targets_uv = targets / torch.tensor(resolution, device="cuda")

    assert torch.all(targets_uv >= 0) and torch.all(targets_uv <= 1)

    uv_texture = torch.zeros(uv_resolution[0], uv_resolution[1], 3, device="cuda")
    target_sampled = sample_texture_nearest(target_image, flip_u(flip_v(targets_uv)))

    uv_texture_0 = targets_uv[:, 0]
    uv_texture_1 = targets_uv[:, 1]
    uv_texture_2 = targets_uv[:, 0]

    uv_texture_all = torch.stack([uv_texture_0, uv_texture_1, uv_texture_2], dim=1)

    uv_texture[uv_center_pixel_id[:, 0].long(), uv_center_pixel_id[:, 1].long()] = (
        target_sampled
    )

    return uv_texture


def plane_board_scene_vertices_and_indices():
    vertices = torch.tensor(
        [
            [-1, -1, 0.0, 0, 0],
            [1.0, -1.0, 0.0, 1, 0],
            [1.0, 1.0, 0.0, 1, 1],
            [-1.0, 1.0, 0.0, 0, 1],
        ]
    ).cuda()
    assert vertices.is_contiguous()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    assert indices.is_contiguous()
    return vertices, indices


import numpy as np
import glints.rasterization as rasterization
import hd_USTC_CG_py


class Renderer:
    def __init__(self, t="lines"):
        self.context = hd_USTC_CG_py.MeshIntersectionContext()
        self.type = t
        if t == "lines":
            self.scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
        else:
            self.scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
        self.world_to_view_matrix = np.eye(4)
        self.view_to_clip_matrix = np.eye(4)
        self.vertex_buffer_stride = 5 * 4
        self.glints_roughness = torch.tensor([0.0016], device="cuda")
        self.scratch_context.set_max_pair_buffer_ratio(20)

    def set_light_position(self, light_position):
        self.light_position = light_position

    def set_camera_position(self, camera_position):
        self.camera_position = camera_position
        self.world_to_view_matrix = rasterization.look_at(
            camera_position, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
        )

    def set_look_at(self, eye, center, up):
        self.camera_position = eye
        self.world_to_view_matrix = rasterization.look_at(eye, center, up)

    def set_perspective(self, fovx, aspect, near, far):
        self.view_to_clip_matrix = rasterization.perspective(fovx, aspect, near, far)

    def set_width(self, width):
        self.width = width

    def set_glints_roughness(self, glints_roughness):
        self.glints_roughness = glints_roughness

    def set_mesh(self, vertices, indices, vertex_buffer_stride=5 * 4):
        self.vertices = vertices
        self.indices = indices
        self.vertex_buffer_stride = vertex_buffer_stride

    def preliminary_render(self, resolution):
        """
        Only get the patches in this step.
        """

        patches, worldToUV, targets = self.context.intersect_mesh_with_rays(
            self.vertices,
            self.indices,
            self.vertex_buffer_stride,
            resolution,
            self.world_to_view_matrix.flatten(),
            self.view_to_clip_matrix.flatten(),
        )

        reshaped_patches = patches.reshape(-1, 4, 2)

        uv_centers = reshaped_patches.mean(dim=1)

        return patches, worldToUV, targets, uv_centers

    def render(self, resolution, lines, force_single_line=False, line_weight=None):
        return render(
            self.context,
            self.scratch_context,
            lines,
            self.width,
            self.glints_roughness,
            self.vertices,
            self.indices,
            self.vertex_buffer_stride,
            resolution,
            self.world_to_view_matrix,
            self.view_to_clip_matrix,
            self.camera_position,
            self.light_position,
            force_single_line,
            line_weight,
        )

    def prepare_target(self, texture_name, uv_resolution):
        return prepare_target(
            texture_name,
            self.context,
            self.vertices,
            self.indices,
            self.vertex_buffer_stride,
            uv_resolution,
            self.world_to_view_matrix,
            self.view_to_clip_matrix,
        )

    def target_bake_to_texture(self, target_name, uv_resolution):
        return target_bake_to_texture(
            target_name,
            self.context,
            self.vertices,
            self.indices,
            self.vertex_buffer_stride,
            uv_resolution,
            self.world_to_view_matrix,
            self.view_to_clip_matrix,
        )
