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
    angle = torch.FloatTensor(count).uniform_(0, 2 * torch.pi).to("cuda")
    x_end = x_start + length * torch.cos(angle)
    y_end = y_start + length * torch.sin(angle)

    lines = torch.zeros((count, 2, 3), device="cuda")
    lines[:, 0, :2] = torch.stack((x_start, y_start), dim=1)
    lines[:, 1, :2] = torch.stack((x_end, y_end), dim=1)

    return lines
