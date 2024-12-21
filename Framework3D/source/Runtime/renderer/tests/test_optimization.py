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


def test_intersect_optimization():

    context = hd_USTC_CG_py.MeshIntersectionContext()
    scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
    scratch_context.set_max_pair_buffer_ratio(12.0)

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
    import glints.renderer as renderer

    lines = test_utils.random_scatter_lines(0.09, 40000, (0, 1), (0, 1))
    width = torch.tensor([0.001], device="cuda")
    glints_roughness = torch.tensor([0.0002], device="cuda")

    lines.requires_grad_(True)

    optimizer = torch.optim.Adam([lines], lr=0.003)

    import matplotlib.pyplot as plt

    losses = []

    for i in range(100):
        optimizer.zero_grad()

        image = renderer.render(
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
        )
        image *= 10

        loss = torch.mean((image - 0.5) ** 2)
        loss.backward()
        optimizer.step()

        losses.append(loss.item())

        # Check torch vmem occupation status
        allocated_memory = torch.cuda.memory_allocated()
        reserved_memory = torch.cuda.memory_reserved()
        print(f"Allocated memory: {allocated_memory / (1024 ** 2):.2f} MB")
        print(f"Reserved memory: {reserved_memory / (1024 ** 2):.2f} MB")
        torch.cuda.empty_cache()
        print(f"Iteration {i}, Loss: {loss.item()}")
        test_utils.save_image(image, resolution, f"optimization_{i}.png")

    # Plot the loss curve
    plt.plot(losses)
    plt.xlabel("Iteration")
    plt.ylabel("Loss")
    plt.title("Loss Curve")
    plt.savefig("loss_curve.png")

    test_utils.save_image(image, resolution, "optimization.png")
