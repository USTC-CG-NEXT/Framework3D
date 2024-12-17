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
