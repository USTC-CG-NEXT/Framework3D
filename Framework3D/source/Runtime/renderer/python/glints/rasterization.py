import torch
import nvdiffrast.torch as dr
import numpy as np

# ----------------------------------------------------------------------------
# Projection and transformation matrix helpers.
# ----------------------------------------------------------------------------


def perspective_projection(near, far, fov):
    aspect = 1.0
    fov = np.radians(fov)
    f = 1.0 / np.tan(fov / 2.0)
    return torch.tensor(
        [
            [f / aspect, 0, 0, 0],
            [0, f, 0, 0],
            [0, 0, (far + near) / (near - far), 2 * far * near / (near - far)],
            [0, 0, -1, 0],
        ],
        dtype=torch.float32,
    ).cuda()


def translate(x, y, z):
    return torch.tensor(
        [[1, 0, 0, x], [0, 1, 0, y], [0, 0, 1, z], [0, 0, 0, 1]],
        dtype=torch.float32,
    ).cuda()


def rotate_x(a):
    s, c = torch.sin(a), torch.cos(a)
    return torch.tensor(
        [[1, 0, 0, 0], [0, c, s, 0], [0, -s, c, 0], [0, 0, 0, 1]],
        dtype=torch.float32,
    ).cuda()


def rotate_y(a):
    s, c = torch.sin(a), torch.cos(a)
    return torch.tensor(
        [[c, 0, s, 0], [0, 1, 0, 0], [-s, 0, c, 0], [0, 0, 0, 1]],
        dtype=torch.float32,
    ).cuda()


# cam_pos shape: [3]
# target shape: [3]
# up shape: [3]
# return shape: [4, 4] tensor
def look_at(cam_pos, target, up):
    f = (target - cam_pos).astype(np.float32)
    f /= np.linalg.norm(f)
    r = np.cross(f, up).astype(np.float32)
    r /= np.linalg.norm(r)
    u = np.cross(r, f).astype(np.float32)
    u /= np.linalg.norm(u)

    m = np.eye(4).astype(np.float32)
    m[0, :3] = r
    m[1, :3] = u
    m[2, :3] = -f
    m[:3, 3] = -m[:3, :3].dot(cam_pos)

    return torch.tensor(m, dtype=torch.float32).cuda()


def get_triangles():
    vertices = torch.tensor(
        [
            [
                [0.0, 0.0, 0.0, 1.0],
                [1.0, 0.0, 0.0, 1.0],
                [0.0, 1.0, 0.0, 1.0],
                [1.0, 1.0, 0.0, 1.0],
            ]
        ],
        dtype=torch.float32,
    ).cuda()
    indices = torch.tensor([[0, 1, 2], [2, 1, 3]], dtype=torch.int32).cuda()
    return vertices, indices
