import hd_USTC_CG_py

import numpy as np


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


import glints.shaderAB as shader
import imageio


# def test_line_intersect_optimization():

#     context = hd_USTC_CG_py.MeshIntersectionContext()
#     scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
#     scratch_context.set_max_pair_buffer_ratio(12.0)

#     import torch
#     import imageio

#     vertices = torch.tensor(
#         [
#             [-1, -1, 0.0, 0, 0],
#             [1.0, -1.0, 0.0, 1, 0],
#             [1.0, 1.0, 0.0, 1, 1],
#             [-1.0, 1.0, 0.0, 0, 1],
#         ]
#     ).cuda()
#     print(vertices.dtype)
#     assert vertices.is_contiguous()
#     indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
#     assert indices.is_contiguous()

#     vertex_buffer_stride = 5 * 4
#     resolution = [1536, 1024]

#     camera_position_np = np.array([2, 2, 3], dtype=np.float32)
#     light_position_np = np.array([2, -2, 3], dtype=np.float32)

#     world_to_view_matrix = look_at(
#         camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
#     )

#     view_to_clip_matrix = perspective(np.pi / 3, 1.0, 0.1, 1000.0)

#     import glints.test_utils as test_utils
#     import glints.renderer as renderer

#     lines = test_utils.random_scatter_lines(0.09, 40000, (0, 1), (0, 1))
#     width = torch.tensor([0.001], device="cuda")
#     glints_roughness = torch.tensor([0.0002], device="cuda")

#     lines.requires_grad_(True)

#     optimizer = torch.optim.Adam([lines], lr=0.003)

#     import matplotlib.pyplot as plt

#     losses = []

#     for i in range(100):
#         optimizer.zero_grad()

#         image = renderer.render(
#             context,
#             scratch_context,
#             lines,
#             width,
#             glints_roughness,
#             vertices,
#             indices,
#             vertex_buffer_stride,
#             resolution,
#             world_to_view_matrix,
#             view_to_clip_matrix,
#             camera_position_np,
#             light_position_np,
#         )
#         image *= 10

#         loss = torch.mean((image - 0.5) ** 2)
#         loss.backward()
#         optimizer.step()

#         losses.append(loss.item())

#         # Check torch vmem occupation status
#         allocated_memory = torch.cuda.memory_allocated()
#         reserved_memory = torch.cuda.memory_reserved()
#         print(f"Allocated memory: {allocated_memory / (1024 ** 2):.2f} MB")
#         print(f"Reserved memory: {reserved_memory / (1024 ** 2):.2f} MB")
#         torch.cuda.empty_cache()
#         print(f"Iteration {i}, Loss: {loss.item()}")
#         test_utils.save_image(image, resolution, f"optimization_{i}.png")

#     # Plot the loss curve
#     plt.plot(losses)
#     plt.xlabel("Iteration")
#     plt.ylabel("Loss")
#     plt.title("Loss Curve")
#     plt.savefig("loss_curve.png")

#     test_utils.save_image(image, resolution, "optimization.png")

import torch

import glints.test_utils as test_utils
import glints.renderer as renderer


def gamma_to_linear(image):
    return image**2.2


def linear_to_gamma(image):
    return image ** (1 / 2.2)


import lpips

lpips_loss_fn = lpips.LPIPS(net="alex").cuda()

import torchvision.transforms.functional as TF


def perceptual_loss(image, target):
    # blur the image

    reshaped_image = image.unsqueeze(0).permute(0, 3, 1, 2)
    reshaped_target = target.unsqueeze(0).permute(0, 3, 1, 2)
    # image = torch.nn.functional.interpolate(
    #     image, size=(256, 256), mode="bilinear", align_corners=False
    # )
    # target = torch.nn.functional.interpolate(
    #     target, size=(256, 256), mode="bilinear", align_corners=False
    # )
    perceptual_loss_value = lpips_loss_fn(reshaped_image, reshaped_target)

    blurred_image = TF.gaussian_blur(image, kernel_size=3, sigma=1.0)
    blurred_target = TF.gaussian_blur(target, kernel_size=3, sigma=1.0)
    mse_loss_value = torch.nn.functional.l1_loss(blurred_image, blurred_target)

    return mse_loss_value, 0.01 * perceptual_loss_value


def loss_function(image, target):
    return perceptual_loss((image), target)


def rotate_postion(postion_np, angle):
    rotation_matrix = np.array(
        [
            [np.cos(angle), -np.sin(angle), 0],
            [np.sin(angle), np.cos(angle), 0],
            [0, 0, 1],
        ],
        dtype=np.float32,
    )
    rotated_camera_position = np.dot(rotation_matrix, postion_np)
    return rotated_camera_position


def straight_bspline_loss(lines):
    length1 = torch.norm(lines[:, 1] - lines[:, 0], dim=1)  # shape [n, 3]
    length2 = torch.norm(lines[:, 1] - lines[:, 2], dim=1)  # shape [n, 3]
    dir1 = (lines[:, 1] - lines[:, 0]) / length1.unsqueeze(1)
    dir2 = (lines[:, 1] - lines[:, 2]) / length2.unsqueeze(1)

    return torch.mean(torch.sum(torch.abs(dir1 - dir2), dim=1))


import os

os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"
import cv2


def test_bspline_intersect_optimization():
    case = "bspline"
    context = hd_USTC_CG_py.MeshIntersectionContext()
    if case == "bspline":
        scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
        random_gen = test_utils.random_scatter_bsplines
    else:
        scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
        random_gen = test_utils.random_scatter_lines

    scratch_context.set_max_pair_buffer_ratio(13.0)

    import torch
    import imageio

    vertices = torch.tensor(
        [
            [-1, -1, 0.0, 0, 0],
            [1.0, -1.0, 0.0, 1, 0],
            [1.0, 1.0, 0.0, 1, 1],
            [-1.0, 1.0, 0.0, 0, 1],
        ]
    ).cuda()
    assert vertices.is_contiguous()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    assert indices.is_contiguous()

    vertex_buffer_stride = 5 * 4
    resolution = [1536, 1024]

    camera_position_np = np.array([4.0, 0, 2.5], dtype=np.float32)
    light_position_np = np.array([4.5, -4, 4], dtype=np.float32)

    fov_in_degrees = 35

    view_to_clip_matrix = perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )

    width = torch.tensor([0.001 * 0.4], device="cuda")
    glints_roughness = torch.tensor([0.0016], device="cuda")

    import matplotlib.pyplot as plt

    max_length = 0.04

    num_light_positions = 16

    random_gen_closure = lambda: random_gen(0.03, 50000, (0, 1), (0, 1))

    for light_pos_id in range(num_light_positions):
        if light_pos_id >= 8:
            continue

        light_rotation_angle = light_pos_id * (2 * np.pi / num_light_positions)
        rotated_light_init_position = rotate_postion(
            light_position_np, light_rotation_angle
        )
        # light_position_torch = torch.tensor(rotated_light_init_position, device="cuda")

        losses = []
        lines = random_gen_closure()

        lines.requires_grad_(True)
        # light_position_torch.requires_grad_(False)

        optimizer = torch.optim.Adam([lines], lr=0.001, betas=(0.9, 0.999), eps=1e-08)
        rnd_pick_target_id = 0

        import os

        os.makedirs(f"light_pos_{light_pos_id}", exist_ok=True)

        exposure = torch.tensor([100.0], device="cuda")
        exposure.requires_grad_(True)
        temperature = 1.0

        with open(f"light_pos_{light_pos_id}/optimization.log", "a") as log_file:

            for i in range(800):
                # if i < 2 or np.random.rand() < last_loss / this_loss:
                #     rnd_pick_target_id = np.random.randint(0, 21)

                rnd_pick_target_id = (rnd_pick_target_id + 1) % 21

                camera_rotate_angle = (rnd_pick_target_id * (30 / 20) - 15) * (
                    np.pi / 180
                )

                rotated_camera_position = rotate_postion(
                    camera_position_np, camera_rotate_angle
                )

                rotated_light_init_position = rotate_postion(
                    light_position_np, camera_rotate_angle
                )

                world_to_view_matrix = look_at(
                    rotated_camera_position,
                    np.array([0.0, 0, 0.0]),
                    np.array([0.0, 0.0, 1.0]),
                )
                target = cv2.imread(
                    f"targets/render_{rnd_pick_target_id:03d}.exr", cv2.IMREAD_UNCHANGED
                )[..., :3]

                target = torch.tensor(target, dtype=torch.float32).cuda()
                target = torch.rot90(target, k=3, dims=[0, 1])

                test_utils.save_image(
                    target, resolution, f"light_pos_{light_pos_id}/target_{i}.png"
                )

                target = target / target.max()

                optimizer.zero_grad()

                image, low_contribution_mask = renderer.render(
                    context,
                    scratch_context,
                    lines,
                    width,
                    glints_roughness,
                    vertices,
                    indices,
                    vertex_buffer_stride,
                    resolution,
                    world_to_view_matrix,
                    view_to_clip_matrix,
                    rotated_camera_position,
                    rotated_light_init_position,
                )
                # if i == 0:
                # blurred_image = torch.nn.functional.avg_pool2d(
                #     image.detach(), 5, stride=1, padding=2
                # ).detach()
                image = image * 100

                # straight_bspline_loss_value = straight_bspline_loss(lines) * 0.001
                mse_loss, perceptual_loss = loss_function(image, target)
                loss = temperature * (
                    mse_loss + perceptual_loss 
                )  #
                loss.backward()

                # Mask out NaN gradients
                with torch.no_grad():
                    for param in optimizer.param_groups[0]["params"]:
                        if param.grad is not None:
                            nan_mask = torch.isnan(param.grad)
                            param.grad[nan_mask] = torch.zeros_like(
                                param.grad[nan_mask]
                            ).uniform_(-0.00001, 0.00001)

                optimizer.step()

                # Clamp lines to max length

                if case == "bspline":
                    length1 = torch.norm(lines[:, 1] - lines[:, 0], dim=1)
                    length2 = torch.norm(lines[:, 1] - lines[:, 2], dim=1)
                    mask1 = length1 > max_length
                    mask2 = length2 > max_length
                    if mask1.any():
                        direction = (lines[mask1, 1] - lines[mask1, 0]) / length1[
                            mask1
                        ].unsqueeze(1)
                        with torch.no_grad():
                            lines[mask1, 0] = lines[mask1, 1] - direction * max_length
                    if mask2.any():
                        direction = (lines[mask2, 1] - lines[mask2, 2]) / length2[
                            mask2
                        ].unsqueeze(1)
                        with torch.no_grad():
                            lines[mask2, 2] = lines[mask2, 1] - direction * max_length
                else:
                    lengths = torch.norm(lines[:, 1] - lines[:, 0], dim=1)
                    mask = lengths > max_length
                    if mask.any():
                        direction = (lines[mask, 1] - lines[mask, 0]) / lengths[
                            mask
                        ].unsqueeze(1)
                        with torch.no_grad():
                            lines[mask, 1] = lines[mask, 0] + direction * max_length

                if i % 20 == 0 and i < 400:
                    with torch.no_grad():
                        lines[low_contribution_mask] = random_gen_closure()[
                            low_contribution_mask
                        ]

                if i == 0:
                    this_loss = loss.item()
                else:
                    last_loss = this_loss
                    this_loss = loss.item()

                losses.append(loss.item() / temperature)

                torch.cuda.empty_cache()

                log_message = (
                    f"light_pos_id {light_pos_id:2d}, Iteration {i:3d}, Loss: {loss.item()/temperature:.6f}, "
                    f"mse_loss: {mse_loss.item():.6f}, perceptual_loss: {perceptual_loss.item():.6f}, "
                    # f"straight_bspline_loss: {straight_bspline_loss_value.item():.6f}"
                )
                print(log_message)
                log_file.write(log_message + "\n")

                test_utils.save_image(
                    image,
                    resolution,
                    f"light_pos_{light_pos_id}/optimization_{i}.exr",
                )
        # Plot the loss curve
        plt.figure(figsize=(10, 6))
        plt.plot(losses, label=f"light_pos_id {light_pos_id}")
        plt.xlabel("Iteration")
        plt.ylabel("Loss")
        plt.title(f"Loss Curve for light_pos_id {light_pos_id}")
        plt.legend()
        plt.savefig(f"light_pos_{light_pos_id}/loss_curve.png")
