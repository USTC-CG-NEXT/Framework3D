import hd_USTC_CG_py

import numpy as np


# world_to_view
def look_at(eye, center, up):
    f = center - eye
    f /= np.linalg.norm(f)
    u = up / np.linalg.norm(up)
    s = np.cross(f, u)
    s /= np.linalg.norm(s)
    u = np.cross(s, f)
    m = np.eye(4)
    m[0, :3] = s
    m[1, :3] = u
    m[2, :3] = -f
    m[0, 3] = -np.dot(s, eye)
    m[1, 3] = -np.dot(u, eye)
    m[2, 3] = np.dot(f, eye)
    return m


# view_to_clip
def perspective(fovy, aspect, near, far):
    f = 1.0 / np.tan(fovy / 2.0)
    m = np.zeros((4, 4))
    m[0, 0] = f / aspect
    m[1, 1] = f
    m[2, 2] = (far + near) / (near - far)
    m[2, 3] = 2.0 * far * near / (near - far)
    m[3, 2] = -1.0
    return m


import glints.shader as shader
import imageio


def save_image(image, resolution, filename):
    # Move the image to CPU and convert to numpy array
    image_cpu = image.cpu().numpy()

    # Rotate the image counterclockwise by 90 degrees
    image_cpu = np.rot90(image_cpu)

    # Save the image using imageio
    imageio.imwrite(filename, (image_cpu * 255).astype(np.uint8))

    # def test_intersect_mesh():
    #     context = hd_USTC_CG_py.MeshIntersectionContext()

    #     import torch

    #     vertices = torch.tensor(
    #         [
    #             [-1, -1, 0.0, 0, 0],
    #             [1.0, -1.0, 0.0, 1, 0],
    #             [1.0, 1.0, 0.0, 1, 1],
    #             [-1.0, 1.0, 0.0, 0, 1],
    #         ]
    #     ).cuda()
    #     assert vertices.is_contiguous()
    #     indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    #     assert indices.is_contiguous()

    #     vertex_buffer_stride = 5 * 4
    #     resolution = [1536 * 2, 1024 * 2]

    #     camera_position_np = np.array([3, 0.5, 3])

    #     world_to_view_matrix = look_at(
    #         np.array([3, 0.5, 3]), np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    #     )

    #     view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

    #     patches, worldToUV, targets = context.intersect_mesh_with_rays(
    #         vertices,
    #         indices,
    #         vertex_buffer_stride,
    #         resolution,
    #         world_to_view_matrix.flatten(),
    #         view_to_clip_matrix.flatten(),
    #     )

    #     print("patches_count", patches.shape[0])

    #     # Create an empty image with the given resolution
    #     image = torch.zeros(
    #         (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
    #     )

    #     # Scatter the first 3 dimensions of patches to the image at the positions specified by targets
    #     image[targets[:, 0].long(), targets[:, 1].long()] = patches[:, :3]

    #     save_image(image, resolution, "uv.png")

    #     scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
    #     scratch_context.set_max_pair_buffer_ratio(10.0)
    #     import glints.test_utils as test_utils

    #     lines = test_utils.random_scatter_lines(0.05, 40000, (-1, 1), (-1, 1))

    #     intersection_pairs = scratch_context.intersect_line_with_rays(lines, patches, 0.001)
    #     print("intersection_pairs count", intersection_pairs.shape[0])
    #     # the result is a buffer of size [intersection_count, 2],
    #     # each element is [line_id, patch_id]

    #     # Create a buffer to accumulate the contributions of intersections
    #     contribution_accumulation = torch.zeros(
    #         (patches.shape[0],), dtype=torch.float32, device="cuda"
    #     )

    #     contribution = torch.ones(intersection_pairs.shape[0], device="cuda")

    #     # Accumulate the contributions of intersections using scatter_add
    #     contribution_accumulation.scatter_add_(
    #         0,
    #         intersection_pairs[:, 1].long(),
    #         contribution,
    #     )

    #     # Scatter the intersection_buffer to the image at the positions specified by targets
    #     image[targets[:, 0].long(), targets[:, 1].long()] = (
    #         contribution_accumulation.unsqueeze(1).expand(-1, 3)
    #     )

    #     save_image(image, resolution, "intersection.png")


import torch


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
        lines, patches, float(width) * 2.0
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


def test_intersect_mesh():

    context = hd_USTC_CG_py.MeshIntersectionContext()
    scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
    scratch_context.set_max_pair_buffer_ratio(15.0)

    import torch
    import imageio

    vertices = torch.tensor(
        [
            [-1, -1, 0.0, 0, 0],
            [1.0, -1.0, 0.0, 1, 0],
            [1.0, 1.0, 0.0, 1, 1],
            [-1.0, 1.0, 0.0, 0, 1],
        ]
    ).cuda()
    print(vertices.dtype)
    assert vertices.is_contiguous()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    assert indices.is_contiguous()

    vertex_buffer_stride = 5 * 4
    resolution = [1536 * 2, 1024 * 2]

    camera_position_np = np.array([2, 2, 3], dtype=np.float32)
    light_position_np = np.array([2, -2, 3], dtype=np.float32)

    world_to_view_matrix = look_at(
        camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )

    view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

    import glints.test_utils as test_utils

    lines = test_utils.random_scatter_lines(0.03, 80000, (0, 1), (0, 1))
    width = torch.tensor([0.001], device="cuda")
    glints_roughness = torch.tensor([0.002], device="cuda")

    numviews = 60
    for i in range(numviews):
        angle = i * (2 * np.pi / numviews)
        rotation_matrix = np.array(
            [
                [np.cos(angle), -np.sin(angle), 0],
                [np.sin(angle), np.cos(angle), 0],
                [0, 0, 1],
            ],
            dtype=np.float32,
        )
        rotated_camera_position = np.dot(rotation_matrix, camera_position_np)

        world_to_view_matrix = look_at(
            rotated_camera_position, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
        )

        image = render(
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
            rotated_camera_position,
            light_position_np,
        )
        image *= 10

        save_image(image, resolution, f"intersection_{i}.png")
