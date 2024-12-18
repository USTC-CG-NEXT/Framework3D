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


def test_intersect_mesh():
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
    resolution = [1536, 1024]

    world_to_view_matrix = look_at(
        np.array([3, 0.5, 3]), np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )
    print(world_to_view_matrix)

    view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

    patches, corners, targets = context.intersect_mesh_with_rays(
        vertices,
        indices,
        vertex_buffer_stride,
        resolution,
        world_to_view_matrix.flatten(),
        view_to_clip_matrix.flatten(),
    )

    print(patches.shape)  # torch.Size([n, 8])
    print(corners.shape)  # torch.Size([n, 9])
    print(targets.shape)  # torch.Size([n, 2]), unsigned int
    print(targets.dtype)  # torch.uint32

    # Create an empty image with the given resolution
    image = torch.zeros(
        (resolution[0], resolution[1], 3), dtype=torch.float32, device="cuda"
    )

    # Scatter the first 3 dimensions of patches to the image at the positions specified by targets
    image[targets[:, 0].long(), targets[:, 1].long()] = patches[:, :3]

    # Move the image to CPU and convert to numpy array
    image_cpu = image.cpu().numpy()

    # Rotate the image counterclockwise by 90 degrees
    image_cpu = np.rot90(image_cpu)

    # Save the image using imageio
    imageio.imwrite("uv.png", (image_cpu * 255).astype(np.uint8))

    scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
    scratch_context.set_max_pair_buffer_ratio(10.0)
    import glints.test_utils as test_utils

    lines = test_utils.random_scatter_lines(0.05, 40000, (-1, 1), (-1, 1))

    intersection_pairs = scratch_context.intersect_line_with_rays(
        lines, patches, 0.001
    )
    # the result is a buffer of size [intersection_count, 2],
    # each element is [line_id, patch_id]

    print(intersection_pairs.shape)

    # Create a buffer to accumulate the contributions of intersections
    intersection_buffer = torch.zeros(
        (patches.shape[0],), dtype=torch.float32, device="cuda"
    )

    # Accumulate the contributions of intersections using scatter_add
    intersection_buffer.scatter_add_(
        0,
        intersection_pairs[:, 1].long(),
        torch.ones(intersection_pairs.shape[0], device="cuda"),
    )

    print(intersection_buffer.shape)
    print(intersection_buffer.dtype)

    # Scatter the intersection_buffer to the image at the positions specified by targets
    image[targets[:, 0].long(), targets[:, 1].long()] = intersection_buffer.unsqueeze(
        1
    ).expand(-1, 3)

    # Move the image to CPU and convert to numpy array
    image_cpu = image.cpu().numpy()

    # Rotate the image counterclockwise by 90 degrees
    image_cpu = np.rot90(image_cpu)

    # Save the image using imageio
    imageio.imwrite("intersection.png", (image_cpu * 255).astype(np.uint8))
