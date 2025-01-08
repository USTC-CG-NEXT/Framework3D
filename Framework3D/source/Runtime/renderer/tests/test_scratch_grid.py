import glints.scratch_grid
import glints.renderer
import glints.test_utils as test_utils
import torch
import numpy as np
import pytest
import glints.bspline as bspline


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


import matplotlib.pyplot as plt


def control_points_to_lines(ctr_points):

    evaluated = []
    for t in torch.linspace(1, 2, 16, device=ctr_points.device):
        evaluated.append(
            bspline.eval_quadratic_bspline_point(
                ctr_points, t.repeat(ctr_points.shape[0])
            )
        )

    evaluated = torch.stack(evaluated, dim=0)

    return evaluated


def sub_test_field(field, arrow_distance, filename, discretize_density=10):

    # test importance sample

    np_sub_field = field.field[:, :, 0, :].detach().cpu().numpy()

    np_sub_density_field = np.linalg.norm(np_sub_field, axis=2)
    np_sub_direction_field = np_sub_field / np_sub_density_field[:, :, None]

    plt.figure(figsize=(10, 10))

    # draw the arrows
    W = field.field.shape[1]
    H = field.field.shape[0]

    plt.quiver(
        np.arange(0, W, arrow_distance),
        np.arange(0, H, arrow_distance),
        np_sub_direction_field[::arrow_distance, ::arrow_distance, 1],
        np_sub_direction_field[::arrow_distance, ::arrow_distance, 0],
    )

    test_points = 0
    if test_points > 0:
        init_points = []

        for i in range(test_points):
            init_point = field._ScratchField__importance_sample_field(
                np_sub_density_field
            )  # np.array([x, y])
            if init_point is not None:
                init_points.append(init_point)
        # do the scatter plot of init_points
        init_points = np.array(init_points)
        plt.scatter(init_points[:, 1], init_points[:, 0], s=1)
        for i in range(test_points):
            grown_curve = field._ScratchField__grow_init_point(
                np_sub_direction_field, np_sub_density_field, init_points[i], 0.2
            )

            plt.plot(grown_curve[:, 1], grown_curve[:, 0], color="red", linewidth=3)

            control_points = field._ScratchField__b_spline_fit(
                grown_curve, max_segments=100, error_tolerance=0.05
            )  # shaped (n,3,2)

            if control_points is not None:
                plotable_curve = control_points_to_lines(torch.tensor(control_points))
                plt.plot(plotable_curve[:, :, 1], plotable_curve[:, :, 0], color="blue")
                # plt.scatter(
                #     control_points[:, 0, 1], control_points[:, 0, 0], color="green"
                # )
                plt.scatter(
                    control_points[:, 1, 1], control_points[:, 1, 0], color="green"
                )
                # plt.scatter(
                #     control_points[:, 2, 1], control_points[:, 2, 0], color="green"
                # )

    else:
        lines = field.discretize_to_lines(discretize_density)
        print("control point count", lines.shape[0])

        plotable_curve = control_points_to_lines(lines).detach().cpu().numpy()
        plt.plot(
            plotable_curve[:, :, 1],
            plotable_curve[:, :, 0],
            color="green",
            linewidth=0.5,
        )

    plt.savefig(filename)
    plt.close()


def optimize_divergence(field):
    field.field.requires_grad = True
    optimizer = torch.optim.Adam([field.field], lr=0.02)
    for i in range(1000):

        optimizer.zero_grad()
        divergence, smoothness = field.calc_divergence_smoothness()
        divergence_loss = torch.nn.functional.smooth_l1_loss(
            divergence, torch.zeros_like(divergence)
        )  # + torch.nn.functional.smooth_l1_loss(smoothness, torch.zeros_like(smoothness))
        divergence_loss.backward()
        optimizer.step()

        # print("divergence_loss", divergence_loss.item())


# @pytest.mark.skip(reason="Skipping temporarily")
def test_scratch_field_discretizing():

    field = glints.scratch_grid.ScratchField(512, 1)

    # case 0: a field with random directions

    random_theta = (
        (torch.rand((512, 512, 5), dtype=torch.float32, device="cuda") - 0.5)
        * 0.8
        * np.pi
    )
    field.field = (
        torch.stack([torch.cos(random_theta), torch.sin(random_theta)], dim=3) * 10
    )

    print("case 0, field shape", field.field.shape)
    sub_test_field(field, 16, "case0.pdf")

    field.field = (
        torch.stack([torch.cos(random_theta), torch.sin(random_theta)], dim=3) * 10
    )

    optimize_divergence(field)

    sub_test_field(field, 16, "case0_optimized.pdf")

    #  case 1: a field with a single direction, all with the same length

    # case 2: a field all pointting to the outside of the image
    pointing_outside_theta = torch.atan2(
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(0),
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(1),
    )
    pointing_outside_length = torch.sqrt(
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(0) ** 2
        + torch.linspace(-256, 255, 512, device="cuda").unsqueeze(1) ** 2
    )
    field.field = (
        torch.stack(
            [
                torch.cos(pointing_outside_theta) * pointing_outside_length,
                torch.sin(pointing_outside_theta) * pointing_outside_length,
            ],
            dim=2,
        )
        .unsqueeze(2)
        .repeat(1, 1, 5, 1)
    )

    print("case 2, field shape", field.field.shape)

    sub_test_field(field, 16, "case2.pdf", discretize_density=40)

    # case 3: a field pointing to the center of the image

    # case 4: a field rotating around the center of the image
    rotating_theta = torch.atan2(
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(0),
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(1),
    )

    density = torch.sqrt(
        torch.linspace(-256, 255, 512, device="cuda").unsqueeze(0) ** 2
        + torch.linspace(-256, 255, 512, device="cuda").unsqueeze(1) ** 2
    )

    rotating_theta = rotating_theta + 0.5 * np.pi
    field.field = (
        torch.stack(
            [
                torch.cos(rotating_theta),
                torch.sin(rotating_theta),
            ],
            dim=2,
        )
        .unsqueeze(2)
        .repeat(1, 1, 5, 1)
    ) * density.unsqueeze(2).unsqueeze(2)

    print("case 4, field shape", field.field.shape)

    sub_test_field(field, 16, "case4.pdf", discretize_density=40)


@pytest.mark.skip(reason="Skipping temporarily")
def test_scratch_field_divergence():
    n = 512
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
    field = glints.scratch_grid.ScratchField(512, 5)

    optimizer = torch.optim.Adam([field.field], lr=0.01)
    for i in range(1000):

        optimizer.zero_grad()
        divergence, smoothness = field.calc_divergence_smoothness()
        divergence_loss = torch.mean(divergence**2)
        divergence_loss.backward()
        optimizer.step()

        print("divergence_loss", divergence_loss.item())

    test_utils.save_image(divergence[:, :, 0], [512, 512], "divergence.exr")

    r = glints.renderer.Renderer()
    vertices, indices = glints.renderer.plane_board_scene_vertices_and_indices()
    camera_position_np = np.array([4.0, 0.1, 2.5], dtype=np.float32)
    r.set_camera_position(camera_position_np)
    fov_in_degrees = 35
    resolution = [1536, 512]
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

    for _ in range(200):
        optimizer.zero_grad()
        image, sampled_mask = glints.scratch_grid.render_scratch_field(
            renderer, resolution, field
        )
        loss_image = loss_fn(linear_to_gamma(image), target_image) * 1000
        density_loss = torch.mean(
            torch.norm(field.field[sampled_mask].reshape(-1, 2), dim=1) * 0.3
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
    resolution = [768 * 2, 256 * 2]
    r.set_perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )
    r.set_mesh(vertices, indices)
    r.set_light_position(torch.tensor([4.0, -0.0, 4.5], device="cuda"))

    r.set_width(torch.tensor([0.001], device="cuda"))

    field = glints.scratch_grid.ScratchField(256, 3)
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

    sub_test_field(field, "scratch_field.pdf")

    test_utils.plot_arrows(
        directions, "directions", spacing=16, scale=0.1, filename="directions.pdf"
    )
