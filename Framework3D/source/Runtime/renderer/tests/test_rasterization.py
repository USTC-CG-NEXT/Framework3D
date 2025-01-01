import hd_USTC_CG_py
import numpy as np
import torch
import glints.shaderAB as shader
import glints.renderer as renderer
import glints.test_utils as test_utils
import glints.rasterization as rasterization
import imageio


def setup_context():
    context = hd_USTC_CG_py.MeshIntersectionContext()
    scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
    return context, scratch_context


def setup_vertices_and_indices():
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


def setup_matrices(camera_position_np, resolution):
    world_to_view_matrix = rasterization.look_at(
        camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )
    view_to_clip_matrix = rasterization.perspective(
        np.pi / 3, resolution[0] / resolution[1], 0.1, 1000.0
    )
    return world_to_view_matrix, view_to_clip_matrix


def save_image(image, resolution, filename):
    test_utils.save_image(image, resolution, filename)


def test_bake_texture():
    context, scratch_context = setup_context()
    scratch_context.set_max_pair_buffer_ratio(10.0)
    vertices, indices = setup_vertices_and_indices()

    vertex_buffer_stride = 5 * 4
    resolution = [1536, 1024]
    camera_position_np = np.array([4.0, 0, 2.5], dtype=np.float32)

    # Need to keep this line since fov is different from other functions
    fov_in_degrees = 35
    world_to_view_matrix, _ = setup_matrices(camera_position_np, resolution)
    view_to_clip_matrix = rasterization.perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )

    uv_resolution = [512, 512]
    uv_texture = renderer.target_bake_to_texture(
        "targets/render_010.exr",
        context,
        vertices,
        indices,
        vertex_buffer_stride,
        uv_resolution,
        world_to_view_matrix,
        view_to_clip_matrix,
    )
    test_utils.save_image(uv_texture, uv_resolution, "baked_texture.exr")


def test_prepare_target_rewrite():
    context, _ = setup_context()
    vertices, indices = setup_vertices_and_indices()
    vertex_buffer_stride = 5 * 4
    resolution = [1536, 1024]
    camera_position_np = np.array([4.0, 0, 2.5], dtype=np.float32)

    # Need to keep this line since fov is different from other functions
    fov_in_degrees = 35
    world_to_view_matrix, _ = setup_matrices(camera_position_np, resolution)
    view_to_clip_matrix = rasterization.perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )
    image = renderer.prepare_target(
        "baked_texture.exr",
        context,
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix,
        view_to_clip_matrix,
    )
    save_image(image, resolution, "rewrite_target.exr")


# def render_image(
#     context,
#     scratch_context,
#     lines,
#     width,
#     glints_roughness,
#     vertices,
#     indices,
#     vertex_buffer_stride,
#     resolution,
#     camera_position_np,
#     light_position_np,
#     numviews,
#     prefix,
# ):
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
#         world_to_view_matrix = rasterization.look_at(
#             rotated_camera_position, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
#         )
#         view_to_clip_matrix = rasterization.perspective(
#             np.pi / 3, resolution[0] / resolution[1], 0.1, 1000.0
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
#         save_image(image, resolution, f"{prefix}_{i}.png")


# def test_rasterize_mesh():
#     context, _ = setup_context()
#     vertices, indices = setup_vertices_and_indices()
#     vertex_buffer_stride = 5 * 4
#     resolution = [1536 * 2, 1024 * 2]
#     camera_position_np = np.array([3, 0.5, 3])
#     world_to_view_matrix, view_to_clip_matrix = setup_matrices(
#         camera_position_np, resolution
#     )

#     patches, worldToUV, targets = context.intersect_mesh_with_rays(
#         vertices,
#         indices,
#         vertex_buffer_stride,
#         resolution,
#         world_to_view_matrix.flatten(),
#         view_to_clip_matrix.flatten(),
#     )

#     print("patches_count", patches.shape[0])

#     image = torch.zeros(
#         (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
#     )
#     image[targets[:, 0].long(), targets[:, 1].long()] = patches[:, :3]
#     save_image(image, resolution, "uv.png")

#     scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
#     scratch_context.set_max_pair_buffer_ratio(10.0)
#     lines = test_utils.random_scatter_lines(0.05, 40000, (-1, 1), (-1, 1))

#     intersection_pairs = scratch_context.intersect_line_with_rays(lines, patches, 0.001)
#     print("intersection_pairs count", intersection_pairs.shape[0])

#     contribution_accumulation = torch.zeros(
#         (patches.shape[0],), dtype=torch.float32, device="cuda"
#     )
#     contribution = torch.ones(intersection_pairs.shape[0], device="cuda")
#     contribution_accumulation.scatter_add_(
#         0, intersection_pairs[:, 1].long(), contribution
#     )
#     image[targets[:, 0].long(), targets[:, 1].long()] = (
#         contribution_accumulation.unsqueeze(1).expand(-1, 3)
#     )
#     save_image(image, resolution, "mesh.png")


# def test_render_directed_scratches():
#     context, scratch_context = setup_context()
#     scratch_context.set_max_pair_buffer_ratio(15.0)
#     lines = test_utils.generate_random_scatter_lines_directed(
#         0.03, 80000, (0, 1), (0, 1), (-5 / 180 * np.pi, 5 / 180 * np.pi)
#     )
#     width = torch.tensor([0.001], device="cuda")
#     glints_roughness = torch.tensor([0.002], device="cuda")

#     vertices, indices = setup_vertices_and_indices()
#     vertex_buffer_stride = 5 * 4
#     resolution = [1536 * 2, 1024 * 2]
#     camera_position_np = np.array([2, 2, 3], dtype=np.float32)
#     light_position_np = np.array([2, -2, 3], dtype=np.float32)

#     render_image(
#         context,
#         scratch_context,
#         lines,
#         width,
#         glints_roughness,
#         vertices,
#         indices,
#         vertex_buffer_stride,
#         resolution,
#         camera_position_np,
#         light_position_np,
#         60,
#         "raster_test/directed_intersection",
#     )


# def test_render_scratches(scratch_type="linear"):
#     context, scratch_context = setup_context()
#     if scratch_type == "bspline":
#         scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
#         scratch_context.set_max_pair_buffer_ratio(20.0)
#         lines = test_utils.random_scatter_bsplines(0.08, 80000, (0, 1), (0, 1))
#         width = torch.tensor([0.0005], device="cuda")
#         glints_roughness = torch.tensor([0.0009], device="cuda")
#     else:
#         scratch_context.set_max_pair_buffer_ratio(15.0)
#         lines = test_utils.random_scatter_lines(0.03, 80000, (0, 1), (0, 1))
#         width = torch.tensor([0.001], device="cuda")
#         glints_roughness = torch.tensor([0.002], device="cuda")

#     vertices, indices = setup_vertices_and_indices()
#     vertex_buffer_stride = 5 * 4
#     resolution = [1536 * 2, 1024 * 2]
#     camera_position_np = np.array([2, 2, 3], dtype=np.float32)
#     light_position_np = np.array([2, -2, 3], dtype=np.float32)

#     render_image(
#         context,
#         scratch_context,
#         lines,
#         width,
#         glints_roughness,
#         vertices,
#         indices,
#         vertex_buffer_stride,
#         resolution,
#         camera_position_np,
#         light_position_np,
#         60,
#         "raster_test/intersection",
#     )


def test_prepare_target():
    context, _ = setup_context()
    vertices, indices = setup_vertices_and_indices()
    vertex_buffer_stride = 5 * 4
    resolution = [1536, 1024]
    camera_position_np = np.array([4.0, 0, 2.5], dtype=np.float32)
    world_to_view_matrix, _ = setup_matrices(
        camera_position_np, resolution
    )
    fov_in_degrees = 35
    view_to_clip_matrix = rasterization.perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )

    image = renderer.prepare_target(
        "texture.png",
        context,
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix,
        view_to_clip_matrix,
    )
    save_image(image, resolution, "target.png")
