import hd_USTC_CG_py
import numpy as np
import torch
import imageio
import glints.rasterization as rasterization


def test_prepare_target():
    context = hd_USTC_CG_py.MeshIntersectionContext()

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
    resolution = [1536*2, 1024*2]

    camera_position_np = np.array([2, 2, 3], dtype=np.float32)
    light_position_np = np.array([2, -2, 3], dtype=np.float32)

    world_to_view_matrix = rasterization.look_at(
        camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )

    view_to_clip_matrix = rasterization.perspective(np.pi / 3, 1.0, 0.1, 1000.0)

    import glints.test_utils as test_utils
    import glints.renderer as renderer

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
    test_utils.save_image(image, resolution, "target.png")
