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


def render_and_save_field(field, resolution, filename):
    r = glints.renderer.Renderer()
    vertices, indices = glints.renderer.plane_board_scene_vertices_and_indices()
    camera_position_np = np.array([4.0, 0.1, 2.5], dtype=np.float32)
    r.set_camera_position(camera_position_np)
    fov_in_degrees = 35
    r.set_perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )
    r.set_mesh(vertices, indices)
    r.set_light_position(torch.tensor([4.0, -0.1, 2.5], device="cuda"))

    r.set_width(torch.tensor([0.001], device="cuda"))

    image, sampled_mask = glints.scratch_grid.render_scratch_field(r, resolution, field)
    test_utils.save_image(image, resolution, filename)


def test_scratch_field_discretizing():
    r = glints.renderer.Renderer()

    

@pytest.mark.skip(reason="Skipping temporarily")
def test_scratch_field_divergence():
    n = 1024
    m = 1
    field = glints.scratch_grid.ScratchField(n, m)
    for scale in [0.1, 0.2, 0.3, 0.4, 0.5]:
        random_theta = (
            torch.rand((n, n, m), dtype=torch.float32, device="cuda") - 0.5
        ) * scale
        field.field = (
            torch.stack([torch.cos(random_theta), torch.sin(random_theta)], dim=3) * 0.5
        )
        divergence, smoothness = field.calc_divergence_smoothness()
        test_utils.save_image(divergence[:, :, 0], [n, n], f"divergence_{scale}.exr")

        print(f"scale: {scale}, divergence: {torch.mean(divergence).item()}")

    for scale in [0.1, 0.2, 0.3, 0.4, 0.5]:
        random_theta = (
            torch.rand((n, n, m), dtype=torch.float32, device="cuda") - 0.5
        ) * scale + (
            torch.rand((n, n, m), dtype=torch.float32, device="cuda") > 0
        ) * torch.pi
        field.field = (
            torch.stack([torch.cos(random_theta), torch.sin(random_theta)], dim=3) * 0.5
        )
        divergence, smoothness = field.calc_divergence_smoothness()
        test_utils.save_image(divergence[:, :, 0], [n, n], f"divergence_{scale}.exr")

        print(f"scale: {scale}, divergence: {torch.mean(divergence).item()}")


@pytest.mark.skip(reason="Skipping temporarily")
def test_scratch_field_divergence_optimization():
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


def optimize_field(
    field,
    renderer,
    resolution,
    target_image,
    loss_fn,
    regularization_loss_fn,
    regularizer,
    optimizer,
):
    old_regularization_loss = None
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

    for _ in range(300):
        optimizer.zero_grad()
        image, sampled_mask = glints.scratch_grid.render_scratch_field(
            renderer, resolution, field
        )
        loss_image = loss_fn(linear_to_gamma(image), target_image) * 1000
        density_loss = torch.mean(
            torch.norm(field.field[sampled_mask].reshape(-1, 2), dim=1) * 0.1
        )
        total_loss = loss_image + density_loss
        total_loss.backward()
        optimizer.step()
        field.fill_masked_holes(sampled_mask)

        resularization_loss = torch.tensor(10000000000000.0)
        regularization_steps = 0

        if True:
            while resularization_loss.item() > old_regularization_loss:
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
                regularization_steps += 1

        print(
            "iteration:",
            _,
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
            "regularization_steps",
            regularization_steps,
        )

    field.fix_direction()


def save_images(field, resolution, divergence, smoothness):
    for i in range(field.field.shape[2]):
        test_utils.save_image(
            1000 * divergence[:, :, i], resolution, f"divergence_{i}.exr"
        )
        test_utils.save_image(
            100 * smoothness[:, :, i], resolution, f"smoothness_{i}.exr"
        )

        density = torch.norm(field.field[:, :, i], dim=2)
        directions = field.field[:, :, i] / density.unsqueeze(2)
        directions = torch.cat(
            [directions, torch.zeros_like(directions[:, :, :1])], dim=2
        )

        test_utils.save_image(directions, resolution, f"directions_{i}.exr")
        test_utils.save_image(density, resolution, f"density_{i}.exr")
        test_utils.save_image(field.field[:, :, i, :1], resolution, f"field_{i}.exr")


@pytest.mark.skip(reason="Skipping temporarily")
def test_render_scratch_field():
    r = glints.renderer.Renderer()

    vertices, indices = glints.renderer.plane_board_scene_vertices_and_indices()
    camera_position_np = np.array([4.0, 0.0, 3.5], dtype=np.float32)
    r.set_camera_position(camera_position_np)
    fov_in_degrees = 35
    resolution = [768 * 2, 512 * 2]
    r.set_perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )
    r.set_mesh(vertices, indices)
    r.set_light_position(torch.tensor([4.0, -0.0, 4.5], device="cuda"))

    r.set_width(torch.tensor([0.001], device="cuda"))

    field = glints.scratch_grid.ScratchField(512, 3)
    image, sampled_mask = glints.scratch_grid.render_scratch_field(r, resolution, field)
    test_utils.save_image(image, resolution, "scratch_field_initial.exr")
    target_image = r.prepare_target("texture.png", resolution)
    loss_fn = torch.nn.L1Loss()
    regularization_loss_fn = torch.nn.L1Loss()
    regularizer = torch.optim.Adam([field.field], lr=0.005)
    optimizer = torch.optim.Adam([field.field], lr=0.01)

    optimize_field(
        field,
        r,
        resolution,
        target_image,
        loss_fn,
        regularization_loss_fn,
        regularizer,
        optimizer,
    )

    divergence, smoothness = field.calc_divergence_smoothness()
    save_images(field, resolution, divergence, smoothness)

    image, sampled_mask = glints.scratch_grid.render_scratch_field(r, resolution, field)
    test_utils.save_image(image, resolution, "scratch_field.exr")

    directions = torch.rot90(field.field[:, :, 0, :2])
    test_utils.plot_arrows(
        directions, "directions", spacing=16, scale=0.1, filename="directions.pdf"
    )
