import hd_USTC_CG_py

import numpy as np


def look_at(eye, center, up):
    f = center - eye
    f /= np.linalg.norm(f)
    up /= np.linalg.norm(up)
    s = np.cross(f, up)
    u = np.cross(s, f)
    m = np.eye(4)
    m[:3, :3] = np.column_stack((s, u, -f))
    m[:3, 3] = -m[:3, :3] @ eye
    return m


def perspective(fovy, aspect, near, far):
    f = 1.0 / np.tan(fovy / 2.0)
    m = np.zeros((4, 4))
    m[0, 0] = f / aspect
    m[1, 1] = f
    m[2, 2] = (far + near) / (near - far)
    m[2, 3] = 2.0 * far * near / (near - far)
    m[3, 2] = -1.0
    return m


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

    view_matrix = look_at(
        np.array([3, 3, 3]), np.array([0.0, 0.0, 0.0]), np.array([0.0, 0.0, 1.0])
    )

    print(view_matrix)
    print(view_matrix.dot(np.array([0.0, 0.0, 1.0, 0.0])))

    projection_matrix = perspective(np.pi / 2, 1.0, 0.1, 10.0)

    # world_to_clip is a list, so after mulitplication, flatten it to a Python List
    world_to_clip = (projection_matrix @ view_matrix).flatten().tolist()

    print(world_to_clip)

    patches, corners, targets = context.intersect_mesh_with_rays(
        vertices, indices, vertex_buffer_stride, resolution, world_to_clip
    )

    print(patches)
    print(corners)
    print(targets)
