import torch
import hd_USTC_CG_py


def test_run():
    context = hd_USTC_CG_py.ScratchIntersectionContext()
    context.set_max_pair_buffer_ratio(10.0)

    lines = torch.tensor([[0.0, 0.0, 0.0], [1.0, 1.0, 0.0]], device="cuda")

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

    result = context.intersect_line_with_rays(lines, patches, 0.5)
    print(result.shape)
    print(result.cpu().numpy())


import glints.test_utils as test_utils


def test_draw_picture():
    import imageio

    context = hd_USTC_CG_py.ScratchIntersectionContext()
    context.set_max_pair_buffer_ratio(10.0)

    lines = test_utils.random_scatter_lines(0.04, 60000, (-1, 1), (-1, 1))
    step = 2.0 / 1024
    patches = test_utils.create_patches(1024, step)

    print(patches.shape)

    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)
    result = context.intersect_line_with_rays(lines, patches, 0.001)

    # the result is a buffer of size [intersection_count, 2],
    # each element is [line_id, patch_id]
    image = torch.zeros((1024, 1024), device="cuda")

    patch_ids = result[:, 1].long()
    image.view(-1).index_add_(
        0, patch_ids, torch.ones_like(patch_ids, device="cuda").float()
    )

    image = image.cpu().numpy()
    imageio.imwrite("output.png", (image * 255).astype("uint8"))

    print(result.shape)
    print(result.cpu().numpy())
