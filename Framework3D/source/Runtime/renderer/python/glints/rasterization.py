import torch
import numpy as np

# ----------------------------------------------------------------------------
# Projection and transformation matrix helpers.
# ----------------------------------------------------------------------------


# view_to_clip
def perspective(fovx, aspect, near, far):
    f = 1.0 / np.tan(fovx / 2.0)
    m = np.zeros((4, 4))
    m[0, 0] = f
    m[1, 1] = f * aspect
    m[2, 2] = (far + near) / (near - far)
    m[2, 3] = 2.0 * far * near / (near - far)
    m[3, 2] = -1.0
    return m

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
