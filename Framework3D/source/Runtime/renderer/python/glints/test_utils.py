import torch


def create_patches(size, step, device="cuda"):
    patches = torch.zeros((size, size, 4, 2), device=device)
    x = torch.linspace(-1, 1 - step, size, device=device)
    y = torch.linspace(-1, 1 - step, size, device=device)
    xv, yv = torch.meshgrid(x, y, indexing="ij")

    patches[:, :, 0] = torch.stack((xv, yv), dim=-1)
    patches[:, :, 1] = torch.stack((xv + step, yv), dim=-1)
    patches[:, :, 2] = torch.stack((xv + step, yv + step), dim=-1)
    patches[:, :, 3] = torch.stack((xv, yv + step), dim=-1)

    return patches.reshape(-1, 4, 2)


# random scatter lines with length 0.1, within the range of [-1, 1] * [-1, 1]
def random_scatter_lines(length, count, width_range, height_range):
    x_start = torch.FloatTensor(count).uniform_(*width_range).to("cuda")
    y_start = torch.FloatTensor(count).uniform_(*height_range).to("cuda")
    z_start = torch.FloatTensor(count).uniform_(-1, 1).to("cuda")
    z_end = z_start
    angle = torch.FloatTensor(count).uniform_(0, 2 * torch.pi).to("cuda")
    x_end = x_start + length * torch.cos(angle)
    y_end = y_start + length * torch.sin(angle)

    lines = torch.zeros((count, 2, 3), device="cuda")
    lines[:, 0, :3] = torch.stack((x_start, y_start, z_start), dim=1)
    lines[:, 1, :3] = torch.stack((x_end, y_end, z_end), dim=1)

    return lines


def generate_random_scatter_lines_directed(
    length, count, width_range, height_range, angle_range
):
    x_start = torch.FloatTensor(count).uniform_(*width_range).to("cuda")
    y_start = torch.FloatTensor(count).uniform_(*height_range).to("cuda")
    z_start = torch.FloatTensor(count).uniform_(-1, 1).to("cuda")
    z_end = z_start
    angle = torch.FloatTensor(count).uniform_(*angle_range).to("cuda")
    x_end = x_start + length * torch.cos(angle)
    y_end = y_start + length * torch.sin(angle)

    lines = torch.zeros((count, 2, 3), device="cuda")
    lines[:, 0, :3] = torch.stack((x_start, y_start, z_start), dim=1)
    lines[:, 1, :3] = torch.stack((x_end, y_end, z_end), dim=1)

    return lines


def random_scatter_bsplines(edge_length, count, width_range, height_range):
    x_start = torch.FloatTensor(count).uniform_(*width_range).to("cuda")
    y_start = torch.FloatTensor(count).uniform_(*height_range).to("cuda")
    angles = torch.FloatTensor(count, 2).uniform_(0, 2 * torch.pi).to("cuda")

    x1 = x_start + edge_length * torch.cos(angles[:, 0])
    y1 = y_start + edge_length * torch.sin(angles[:, 0])
    z = torch.FloatTensor(count).uniform_(-1, 1).to("cuda")
    x2 = x_start + edge_length * torch.cos(angles[:, 1])
    y2 = y_start + edge_length * torch.sin(angles[:, 1])

    triangles = torch.zeros((count, 3, 3), device="cuda")
    triangles[:, 0, :3] = torch.stack((x1, y1, z), dim=1)
    triangles[:, 1, :3] = torch.stack((x_start, y_start, z), dim=1)
    triangles[:, 2, :3] = torch.stack((x2, y2, z), dim=1)

    return triangles


import numpy as np
import imageio
import os


def save_image(image, resolution, filename):
    # Create the directory if it does not exist
    if os.path.dirname(filename):
        os.makedirs(os.path.dirname(filename), exist_ok=True)

    # Move the image to CPU and convert to numpy array
    image_cpu = image.detach().cpu().numpy()

    # Clamp the image values to be between 0 and 1

    # Rotate the image counterclockwise by 90 degrees
    image_cpu = np.rot90(image_cpu)

    # Save the image using imageio
    if filename.endswith(".exr"):
        imageio.imwrite(filename, image_cpu.astype(np.float32))
    else:
        image_cpu = np.clip(image_cpu, 0, 1)
        imageio.imwrite(filename, (image_cpu * 255).astype(np.uint8))


import os

os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"
import cv2


def read_image(filename):
    if filename.endswith(".exr"):
        # Read EXR image using OpenCV
        image = cv2.imread(filename, cv2.IMREAD_UNCHANGED)[..., :3]
        image_tensor = torch.tensor(image, dtype=torch.float32).cuda()
    else:
        # Read regular image using imageio
        image = imageio.imread(filename)
        image_tensor = torch.from_numpy(image).float()
        # Normalize regular images to [0,1]
        image_tensor /= 255.0

    # Rotate the image clockwise by 90 degrees
    image_tensor = torch.rot90(image_tensor, k=1, dims=(0, 1))

    return image_tensor


import matplotlib.pyplot as plt


def plot_arrows(tensor, title, spacing=8, filename=None):
    n = tensor.shape[0]
    X, Y = np.meshgrid(np.arange(0, n, spacing), np.arange(0, n, spacing))
    U = tensor[::spacing, ::spacing, 0].detach().cpu().numpy()
    V = tensor[::spacing, ::spacing, 1].detach().cpu().numpy()

    plt.figure(figsize=(10, 10))
    plt.quiver(X, Y, U, V, scale_units="xy")
    plt.title(title)
    plt.gca().invert_yaxis()

    if filename is not None:
        plt.savefig(filename)
    else:
        plt.show()


def rotate_postion(position_np, angle, axis=np.array([0, 0, 1], dtype=np.float32)):
    axis = axis / np.linalg.norm(axis)
    cos_angle = np.cos(angle)
    sin_angle = np.sin(angle)
    one_minus_cos = 1 - cos_angle

    rotation_matrix = np.array(
        [
            [
                cos_angle + axis[0] * axis[0] * one_minus_cos,
                axis[0] * axis[1] * one_minus_cos - axis[2] * sin_angle,
                axis[0] * axis[2] * one_minus_cos + axis[1] * sin_angle,
            ],
            [
                axis[1] * axis[0] * one_minus_cos + axis[2] * sin_angle,
                cos_angle + axis[1] * axis[1] * one_minus_cos,
                axis[1] * axis[2] * one_minus_cos - axis[0] * sin_angle,
            ],
            [
                axis[2] * axis[0] * one_minus_cos - axis[1] * sin_angle,
                axis[2] * axis[1] * one_minus_cos + axis[0] * sin_angle,
                cos_angle + axis[2] * axis[2] * one_minus_cos,
            ],
        ],
        dtype=np.float32,
    )

    rotated_position = np.dot(rotation_matrix, position_np)
    return rotated_position
