import hd_USTC_CG_py


def test_intersect_mesh():
    context = hd_USTC_CG_py.MeshIntersectionContext()

    import torch

    vertices = torch.tensor(
        [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0]]
    ).cuda()
    print(vertices.dtype)
    assert vertices.is_contiguous()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    assert indices.is_contiguous()

    vertex_buffer_stride = 3 * 4
    resolution = [1024, 1024]
    world_to_clip = [
        1.0,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0,
    ]

    result = context.intersect_mesh_with_rays(
        vertices, indices, vertex_buffer_stride, resolution, world_to_clip
    )
    print(result[0].shape)
    print(result[1].shape)
    print(result[2].shape)
    print(result[0].cpu().numpy())
    print(result[1].cpu().numpy())
    print(result[2].cpu().numpy())
