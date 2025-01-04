import glints.scratch_grid
import glints.renderer
import glints.test_utils as test_utils
import torch
import numpy as np
import pytest


def test_scratch_field():
    field = glints.scratch_grid.ScratchField(10, 5)
    assert field.n == 10
    assert field.m == 5
    assert field.field.shape == (10, 10, 5, 2)


def linear_to_gamma(image):
    return image ** (1.0 / 2.2)


@pytest.mark.skip(reason="Skipping temporarily")
def test_scratch_field_divergence():
    field = glints.scratch_grid.ScratchField(1024, 5)

    optimizer = torch.optim.Adam([field.field], lr=0.01)
    for i in range(1000):

        optimizer.zero_grad()
        divergence, smoothness = field.calc_divergence_smoothness()
        divergence_loss = torch.mean(divergence**2)
        divergence_loss.backward()
        optimizer.step()

        print("divergence_loss", divergence_loss.item())

    test_utils.save_image(divergence[:, :, 0], [1024, 1024], "divergence.exr")

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

    image = glints.scratch_grid.render_scratch_field(r, resolution, field)

    test_utils.save_image(image, resolution, "test_divergence_scratch_field.exr")


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

    field = glints.scratch_grid.ScratchField(2048, 2)
    image = glints.scratch_grid.render_scratch_field(r, resolution, field)
    test_utils.save_image(image, resolution, "scratch_field_initial.exr")
    target_image = r.prepare_target("texture.png", resolution)
    loss_fn = torch.nn.L1Loss()
    regularization_loss_fn = torch.nn.L1Loss()
    regularizer = torch.optim.Adam([field.field], lr=0.002)

    for i in range(400):

        regularizer.zero_grad()
        divergence, smoothness = field.calc_divergence_smoothness()

        loss_divergence = regularization_loss_fn(
            divergence, torch.zeros_like(divergence)
        )

        loss_smoothness = regularization_loss_fn(
            smoothness, torch.zeros_like(smoothness)
        )

        resularization_loss = loss_divergence + loss_smoothness
        if i == 0:
            old_regularization_loss = resularization_loss.item()
        resularization_loss.backward()

        regularizer.step()
        field.fix_direction()
    optimizer = torch.optim.Adam([field.field], lr=0.04)
    for _ in range(150):  # Number of optimization steps

        optimizer.zero_grad()
        image = glints.scratch_grid.render_scratch_field(r, resolution, field)
        loss_image = loss_fn(linear_to_gamma(image), target_image) * 1000
        density_loss = torch.mean(
            torch.norm(field.field, dim=3) * 0.1
        )  # less scratches, better direction

        total_loss = loss_image + density_loss
        total_loss.backward()
        optimizer.step()

        resularization_loss = torch.tensor(12.0)

        while resularization_loss.item() > old_regularization_loss * 0.001:
            # for i in range(30):

            regularizer.zero_grad()
            divergence, smoothness = field.calc_divergence_smoothness()
            loss_divergence = regularization_loss_fn(
                divergence, torch.zeros_like(divergence)
            )

            loss_smoothness = regularization_loss_fn(
                smoothness, torch.zeros_like(smoothness)
            )

            resularization_loss = loss_divergence + loss_smoothness
            resularization_loss.backward()

            regularizer.step()


        print(
            "loss_divergence",
            loss_divergence.item(),
            "loss_smoothness",
            loss_smoothness.item(),
            "density_loss",
            density_loss.item(),
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

        density = torch.norm(field.field[:, :, i], dim=2)

        directions = field.field[:, :, i] / density.unsqueeze(2)  # shape [n,n,2]
        # expand 1 dimension to 3 dimensions
        directions = torch.cat(
            [directions, torch.zeros_like(directions[:, :, :1])], dim=2
        )
        test_utils.save_image(directions, resolution, f"directions_{i}.exr")

        test_utils.save_image(density, resolution, f"density_{i}.exr")
        test_utils.save_image(field.field[:, :, i, :1], resolution, f"field_{i}.exr")

    image = glints.scratch_grid.render_scratch_field(r, resolution, field)

    test_utils.save_image(image, resolution, "scratch_field.exr")
