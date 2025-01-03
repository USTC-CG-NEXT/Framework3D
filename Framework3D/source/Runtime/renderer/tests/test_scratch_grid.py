import glints.scratch_grid
import glints.renderer
import glints.test_utils as test_utils
import torch
import numpy as np


def test_scratch_field():
    field = glints.scratch_grid.ScratchField(10, 5)
    assert field.n == 10
    assert field.m == 5
    assert field.field.shape == (10, 10, 5, 2)


def linear_to_gamma(image):
    return image ** (1.0 / 2.2)


def test_render_scratch_field():
    r = glints.renderer.Renderer()

    vertices, indices = glints.renderer.plane_board_scene_vertices_and_indices()
    camera_position_np = np.array([4.0, 0.1, 2.5], dtype=np.float32)
    r.set_camera_position(camera_position_np)
    fov_in_degrees = 35
    resolution = [1536, 1024]
    r.set_perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )
    r.set_mesh(vertices, indices)
    r.set_light_position(torch.tensor([4.0, -0.1, 2.5], device="cuda"))

    r.set_width(torch.tensor([0.001], device="cuda"))

    field = glints.scratch_grid.ScratchField(1024, 1)

    divergence = field.calc_divergence()

    optimizer = torch.optim.Adam([field.field], lr=0.1)
    loss_fn = torch.nn.MSELoss()

    image = glints.scratch_grid.render_scratch_field(r, resolution, field)

    test_utils.save_image(image * 5, resolution, "scratch_field_initial.exr")

    target_image = r.prepare_target("texture.png", resolution)

    for _ in range(200):  # Number of optimization steps
        optimizer.zero_grad()
        divergence = field.calc_divergence()
        loss_divergence = loss_fn(divergence, torch.zeros_like(divergence)) * 1e-8

        smoothness = field.calc_smoothness()
        loss_smoothness = loss_fn(smoothness, torch.zeros_like(smoothness)) * 1e-5

        image = glints.scratch_grid.render_scratch_field(r, resolution, field)
        loss_image = loss_fn(linear_to_gamma(image), target_image)

        total_loss = loss_divergence + loss_smoothness + loss_image
        total_loss.backward()
        optimizer.step()

        print(
            "loss_divergence",
            loss_divergence.item(),
            "loss_smoothness",
            loss_smoothness.item(),
            "loss_image",
            loss_image.item(),
            "total_loss",
            total_loss.item(),
        )

    for i in range(1):
        test_utils.save_image(
            1000 * divergence[:, :, i], resolution, f"divergence_{i}.exr"
        )
        test_utils.save_image(
            100 * smoothness[:, :, i], resolution, f"smoothness_{i}.exr"
        )
        test_utils.save_image(field.field[:, :, i, :1], resolution, f"field_{i}.exr")

    image = glints.scratch_grid.render_scratch_field(r, resolution, field)

    test_utils.save_image(image * 5, resolution, "scratch_field.exr")
