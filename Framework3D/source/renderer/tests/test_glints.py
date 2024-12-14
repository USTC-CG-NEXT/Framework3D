import torch
import hd_USTC_CG_py


# def test_run():
#     context = hd_USTC_CG_py.ScratchIntersectionContext()
#     context.set_max_pair_buffer_ratio(10.0)

#     lines = torch.tensor([[[0.0, 0.0, 0.0], [1.0, 1.0, 0.0]]], device="cuda")

#     patches = torch.zeros((1024, 1024, 4 , 2), device="cuda")

#     step = 2.0 / 1024
#     x = torch.linspace(-1, 1 - step, 1024, device="cuda")
#     y = torch.linspace(-1, 1 - step, 1024, device="cuda")
#     xv, yv = torch.meshgrid(x, y, indexing="ij")

#     patches[:, :, 0] = torch.stack((xv, yv), dim=-1)
#     patches[:, :, 1] = torch.stack((xv + step, yv), dim=-1)
#     patches[:, :, 2] = torch.stack((xv + step, yv + step), dim=-1)
#     patches[:, :, 3] = torch.stack((xv, yv + step), dim=-1)

#     patches = patches.reshape(-1, 4, 2)
#     print(patches.shape)

#     result = context.intersect_line_with_rays(lines, patches, 0.5)
#     print(result.shape)
#     print(result.cpu().numpy())


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


def test_draw_picture():
    import imageio

    context = hd_USTC_CG_py.ScratchIntersectionContext()
    context.set_max_pair_buffer_ratio(10.0)

    lines = random_scatter_lines(0.1, 20000, (-1, 1), (-1, 1))
    patches = torch.zeros((1024, 1024, 4, 2), device="cuda")

    step = 2.0 / 1024
    x = torch.linspace(-1, 1 - step, 1024, device="cuda")
    y = torch.linspace(-1, 1 - step, 1024, device="cuda")
    xv, yv = torch.meshgrid(x, y, indexing="ij")

    patches[:, :, 0] = torch.stack((xv, yv), dim=-1)
    patches[:, :, 1] = torch.stack((xv + step, yv), dim=-1)
    patches[:, :, 2] = torch.stack((xv + step, yv + step), dim=-1)
    patches[:, :, 3] = torch.stack((xv, yv + step), dim=-1)

    patches = patches.reshape(-1, 4, 2)
    print(patches.shape)

    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)

    # the result is a buffewr of size [intersection_count, 2],
    # each element is [line_id, patch_id]
    image = torch.zeros((1024, 1024), device="cuda")

    patch_ids = result[:, 1].long()
    image.view(-1).index_add_(0, patch_ids, torch.ones_like(patch_ids, device="cuda").float())

    image = image.cpu().numpy()
    imageio.imwrite("output.png", (image * 255).astype("uint8"))

    print(result.shape)

    print(result.cpu().numpy())
