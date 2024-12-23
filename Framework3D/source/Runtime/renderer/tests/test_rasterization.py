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
import glints.renderer as renderer
import glints.test_utils as test_utils

import imageio


# def test_rasterize_mesh():
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

#     test_utils.save_image(image, resolution, "uv.png")

#     scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
#     scratch_context.set_max_pair_buffer_ratio(10.0)

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

#     test_utils.save_image(image, resolution, "mesh.png")


# def test_render_linear_scratches():
#     context = hd_USTC_CG_py.MeshIntersectionContext()
#     scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
#     scratch_context.set_max_pair_buffer_ratio(15.0)

#     import torch
#     import imageio

#     vertices = torch.tensor(
#         [
#             [-1, -1, 0.0, 0, 0],
#             [1.0, -1.0, 0.0, 1, 0],
#             [1.0, 1.0, 0.0, 1, 1],
#             [-1.0, 1.0, 0.0, 0, 1],
#         ]
#     ).cuda()
#     print(vertices.dtype)
#     assert vertices.is_contiguous()
#     indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
#     assert indices.is_contiguous()

#     vertex_buffer_stride = 5 * 4
#     resolution = [1536 * 2, 1024 * 2]

#     camera_position_np = np.array([2, 2, 3], dtype=np.float32)
#     light_position_np = np.array([2, -2, 3], dtype=np.float32)

#     world_to_view_matrix = look_at(
#         camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
#     )

#     view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

#     import glints.test_utils as test_utils

#     lines = test_utils.random_scatter_lines(0.03, 80000, (0, 1), (0, 1))

#     width = torch.tensor([0.001], device="cuda")
#     glints_roughness = torch.tensor([0.002], device="cuda")

#     numviews = 60
#     for i in range(numviews):
#         angle = i * (2 * np.pi / numviews)
#         rotation_matrix = np.array(
#             [
#                 [np.cos(angle), -np.sin(angle), 0],
#                 [np.sin(angle), np.cos(angle), 0],
#                 [0, 0, 1],
#             ],
#             dtype=np.float32,
#         )
#         rotated_camera_position = np.dot(rotation_matrix, camera_position_np)

#         world_to_view_matrix = look_at(
#             rotated_camera_position, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
#         )

#         image, _ = renderer.render(
#             context,
#             scratch_context,
#             lines,
#             width,
#             glints_roughness,
#             vertices,
#             indices,
#             vertex_buffer_stride,
#             resolution,
#             world_to_view_matrix,
#             view_to_clip_matrix,
#             rotated_camera_position,
#             light_position_np,
#         )
#         image *= 10

#         test_utils.save_image(image, resolution, f"intersection_{i}.png")


def test_render_bspline_scratches():
    context = hd_USTC_CG_py.MeshIntersectionContext()
    scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
    scratch_context.set_max_pair_buffer_ratio(20.0)

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
    resolution = [1536, 1024]

    camera_position_np = np.array([2, 2, 3], dtype=np.float32)
    light_position_np = np.array([2, -2, 3], dtype=np.float32)

    world_to_view_matrix = look_at(
        camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )

    view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

    import glints.test_utils as test_utils

    lines = test_utils.random_scatter_bsplines(0.08, 80000, (0, 1), (0, 1))

    width = torch.tensor([0.0005], device="cuda")
    glints_roughness = torch.tensor([0.0009], device="cuda")

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

        image, _ = renderer.render(
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

        test_utils.save_image(image, resolution, f"raster_test/intersection_{i}.png")


# def test_prepare_target():
#     context = hd_USTC_CG_py.MeshIntersectionContext()

#     import torch
#     import imageio

#     vertices = torch.tensor(
#         [
#             [-1, -1, 0.0, 0, 0],
#             [1.0, -1.0, 0.0, 1, 0],
#             [1.0, 1.0, 0.0, 1, 1],
#             [-1.0, 1.0, 0.0, 0, 1],
#         ]
#     ).cuda()
#     print(vertices.dtype)
#     assert vertices.is_contiguous()
#     indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
#     assert indices.is_contiguous()

#     vertex_buffer_stride = 5 * 4
#     resolution = [1536, 1024]

#     camera_position_np = np.array([2, 2, 3], dtype=np.float32)
#     light_position_np = np.array([2, -2, 3], dtype=np.float32)

#     world_to_view_matrix = look_at(
#         camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
#     )

#     view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

#     import glints.test_utils as test_utils
#     import glints.renderer as renderer

#     image = renderer.prepare_target(
#         "texture.png",
#         context,
#         vertices,
#         indices,
#         vertex_buffer_stride,
#         resolution,
#         world_to_view_matrix,
#         view_to_clip_matrix,
#     )
#     test_utils.save_image(image, resolution, "target.png")
