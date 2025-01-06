import hd_USTC_CG_py

import numpy as np
import glints.shaderAB as shader
import imageio

import torch

import glints.test_utils as test_utils
import glints.renderer as renderer
import glints.bspline as bspline
import matplotlib.pyplot as plt


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


def gamma_to_linear(image):
    return image**2.2


def linear_to_gamma(image):
    return image ** (1 / 2.2)


import lpips

lpips_loss_fn = lpips.LPIPS(net="alex").cuda()

import torchvision.transforms.functional as TF

import glints.bspline


def perceptual_loss(image, target):

    reshaped_image = image.unsqueeze(0).permute(0, 3, 1, 2)
    reshaped_target = target.unsqueeze(0).permute(0, 3, 1, 2)

    perceptual_loss_value = lpips_loss_fn(reshaped_image, reshaped_target)

    blurred_image = TF.gaussian_blur(image, kernel_size=3, sigma=1.0)
    blurred_target = TF.gaussian_blur(target, kernel_size=3, sigma=1.0)
    mse_loss_value = torch.nn.functional.l1_loss(blurred_image, blurred_target)

    return mse_loss_value, 0.01 * perceptual_loss_value


def loss_function(image, target):
    return perceptual_loss((image), target)


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


def straight_bspline_loss(lines):
    length1 = torch.norm(lines[:, 1] - lines[:, 0], dim=1)  # shape [n, 3]
    length2 = torch.norm(lines[:, 1] - lines[:, 2], dim=1)  # shape [n, 3]
    dir1 = (lines[:, 1] - lines[:, 0]) / length1.unsqueeze(1)
    dir2 = (lines[:, 1] - lines[:, 2]) / length2.unsqueeze(1)

    return torch.mean(torch.sum(torch.abs(dir1 - dir2), dim=1))


def to_luminance(image):
    return 0.2126 * image[..., 0] + 0.7152 * image[..., 1] + 0.0722 * image[..., 2]


def initilize_based_on_target(targets, edge_length, count, width_range, height_range):

    all_triangles = []
    for i in range(21):
        test_utils.save_image(targets[i], [1024, 1024], f"uv_baked/baked_{i:03d}.exr")

    num_points_per_target = count // len(targets)
    for target in targets:
        # Calculate CDF for current target
        target_luminance = to_luminance(target)

        # the mask decrease from left to right by coordinate, from 0.8 to 0.2 linearly, shape [1024, 1024]
        brightness_left_to_right_decrease_mask = (
            torch.linspace(1.0, 0.0, 1024).unsqueeze(1).expand(1024, 1024).cuda()
        )

        target_luminance = target_luminance  #  * brightness_left_to_right_decrease_mask

        flat_pdf = target_luminance.T.flatten()
        cdf = torch.cumsum(flat_pdf, dim=0)
        cdf = cdf / cdf[-1]

        # Generate points for current target
        random_values = (
            torch.FloatTensor(num_points_per_target, 1).uniform_(0, 1).to("cuda")
        )
        flat_indices = torch.searchsorted(cdf, random_values).squeeze()

        H, W = target.shape[:2]
        y_indices = flat_indices // W
        x_indices = flat_indices % W

        x_start = (x_indices.float() / W) * (
            width_range[1] - width_range[0]
        ) + width_range[0]
        y_start = (y_indices.float() / H) * (
            height_range[1] - height_range[0]
        ) + height_range[0]
        z_start = torch.FloatTensor(num_points_per_target).uniform_(-1, 1).to("cuda")
        angles = (
            torch.FloatTensor(num_points_per_target, 2)
            .uniform_(0, 2 * torch.pi)
            .to("cuda")
        )

        x1 = x_start + edge_length * torch.cos(angles[:, 0])
        y1 = y_start + edge_length * torch.sin(angles[:, 0])
        z1 = torch.FloatTensor(num_points_per_target).uniform_(-1, 1).to("cuda")
        z2 = z1
        x2 = x_start + edge_length * torch.cos(angles[:, 1])
        y2 = y_start + edge_length * torch.sin(angles[:, 1])

        target_triangles = torch.zeros((num_points_per_target, 3, 3), device="cuda")
        target_triangles[:, 0, :3] = torch.stack((x1, y1, z1), dim=1)
        target_triangles[:, 1, :3] = torch.stack((x_start, y_start, z_start), dim=1)
        target_triangles[:, 2, :3] = torch.stack((x2, y2, z2), dim=1)

        all_triangles.append(target_triangles.clone())

    return torch.cat(all_triangles, dim=0).contiguous()  # shaped [count, 3, 3]


import os

os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"
import cv2


def fix_max_length(lines, max_length, case):
    if case == "bspline":
        length1 = torch.norm(lines[:, 1] - lines[:, 0], dim=1)
        length2 = torch.norm(lines[:, 1] - lines[:, 2], dim=1)
        mask1 = length1 > max_length
        mask2 = length2 > max_length
        if mask1.any():
            direction = (lines[mask1, 1] - lines[mask1, 0]) / length1[mask1].unsqueeze(
                1
            )
            with torch.no_grad():
                lines[mask1, 0] = lines[mask1, 1] - direction * max_length
        if mask2.any():
            direction = (lines[mask2, 1] - lines[mask2, 2]) / length2[mask2].unsqueeze(
                1
            )
            with torch.no_grad():
                lines[mask2, 2] = lines[mask2, 1] - direction * max_length
    else:
        lengths = torch.norm(lines[:, 1] - lines[:, 0], dim=1)
        mask = lengths > max_length
        if mask.any():
            direction = (lines[mask, 1] - lines[mask, 0]) / lengths[mask].unsqueeze(1)
            with torch.no_grad():
                lines[mask, 1] = lines[mask, 0] + direction * max_length
    return lines


def redistribute_low_contribution_points(lines, low_contribution_mask, random_line_gen):
    with torch.no_grad():
        if low_contribution_mask.any():
            lines[low_contribution_mask] = random_line_gen()[low_contribution_mask]
        return lines


def test_bspline_intersect_optimization():
    case = "bspline"
    context = hd_USTC_CG_py.MeshIntersectionContext()
    if case == "bspline":
        scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
        random_gen = test_utils.random_scatter_bsplines
    else:
        scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
        random_gen = test_utils.random_scatter_lines

    scratch_context.set_max_pair_buffer_ratio(30.0)

    vertices = torch.tensor(
        [
            [-1, -1, 0.0, 0, 0],
            [1.0, -1.0, 0.0, 1, 0],
            [1.0, 1.0, 0.0, 1, 1],
            [-1.0, 1.0, 0.0, 0, 1],
        ]
    ).cuda()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()

    vertex_buffer_stride = 5 * 4
    resolution = [768, 512]

    camera_position_np = np.array([0.0, 0, 5.0], dtype=np.float32)
    light_position_np = np.array([8.0, 0.0, 8], dtype=np.float32)

    fov_in_degrees = 35
    view_to_clip_matrix = perspective(
        np.pi * fov_in_degrees / 180.0, resolution[0] / resolution[1], 0.1, 1000.0
    )

    width = torch.tensor([0.001], device="cuda")
    glints_roughness = torch.tensor([0.0016], device="cuda")

    max_length = 0.2
    num_light_positions = 16

    all_target_max = torch.tensor(0.0, device="cuda")
    targets = []
    for i in range(21):
        target = cv2.imread(f"targets/render_{i:03d}.exr", cv2.IMREAD_UNCHANGED)[
            ..., :3
        ]
        target = torch.tensor(target, dtype=torch.float32).cuda()
        target = torch.rot90(target, k=3, dims=[0, 1])
        targets.append(target)
        all_target_max = torch.max(target.max(), all_target_max)

    uv_resolution = [1024, 1024]
    baked_textures = []
    for i in range(21):
        camera_rotate_angle = (i * (20.0 / 20) - 10.0) * (np.pi / 180)
        rotated_camera_position = rotate_postion(
            camera_position_np,
            camera_rotate_angle,
            axis=np.array([-1, 0, 0], dtype=np.float32),
        )
        world_to_view_matrix = look_at(
            rotated_camera_position,
            np.array([0.0, 0, 0.0]),
            np.array([-1.0, 0.0, 0.0]),
        )
        baked = renderer.target_bake_to_texture(
            torch.rot90(targets[i], k=2, dims=[0, 1]),
            context,
            vertices,
            indices,
            vertex_buffer_stride,
            uv_resolution,
            world_to_view_matrix,
            view_to_clip_matrix,
        )
        baked_textures.append(baked.clone())

    for i in range(1, 21):
        test_utils.save_image(
            baked_textures[i], uv_resolution, f"baked_texture_{i}.exr"
        )

    random_gen_closure = lambda: initilize_based_on_target(
        baked_textures, 0.1, 15000, (0, 1), (0, 1)
    )
    for light_pos_id in range(num_light_positions):

        light_rotation_angle = light_pos_id * (2 * np.pi / num_light_positions)

        losses = []
        lines = random_gen_closure().clone().contiguous().cuda()
        lines = fix_max_length(lines, max_length, case)
        lines.requires_grad_(True)

        optimizer = torch.optim.Adam([lines], lr=0.00005, betas=(0.9, 0.999), eps=1e-08)
        iterative_rnd_pick_target_id = 10

        os.makedirs(f"light_pos_{light_pos_id}", exist_ok=True)

        exposure = torch.tensor([1.0], device="cuda")
        exposure.requires_grad_(True)
        temperature = 1.0

        with open(f"light_pos_{light_pos_id}/optimization.log", "a") as log_file:
            for i in range(300):
                rnd_pick_target_ids = np.random.randint(0, 21, size=5)
                iterative_rnd_pick_target_id = (iterative_rnd_pick_target_id + 1) % 21
                rnd_pick_target_ids[-1] = iterative_rnd_pick_target_id

                total_loss = 0
                for rnd_pick_target_id in rnd_pick_target_ids:
                    camera_rotate_angle = (rnd_pick_target_id * (20.0 / 20) - 10) * (
                        np.pi / 180
                    )
                    rotated_camera_position = rotate_postion(
                        camera_position_np,
                        camera_rotate_angle,
                        axis=np.array([-1, 0, 0], dtype=np.float32),
                    )
                    rotated_light_position = rotate_postion(
                        light_position_np,
                        light_rotation_angle,
                        axis=np.array([0, 0, 1]),
                    )
                    light_position_torch = torch.tensor(
                        rotated_light_position, device="cuda"
                    )
                    world_to_view_matrix = look_at(
                        rotated_camera_position,
                        np.array([0.0, 0, 0.0]),
                        np.array([-1.0, 0.0, 0.0]),
                    )
                    target = targets[rnd_pick_target_id] / all_target_max

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
                        light_position_torch,
                    )

                    image = image * 150

                    straight_bspline_loss_value = straight_bspline_loss(lines) * 0.001
                    mse_loss, perceptual_loss = loss_function(image, target)

                    loss = temperature * (mse_loss + perceptual_loss)
                    if case == "bspline":
                        loss += straight_bspline_loss_value
                    total_loss += loss

                total_loss.backward()

                with torch.no_grad():
                    for param in optimizer.param_groups[0]["params"]:
                        if param.grad is not None:
                            nan_mask = torch.isnan(param.grad)
                            param.grad[nan_mask] = torch.zeros_like(
                                param.grad[nan_mask]
                            ).uniform_(-0.00001, 0.00001)

                optimizer.step()

                if i < 100 and i % 5 == 0:
                    lines = redistribute_low_contribution_points(
                        lines, low_contribution_mask, random_gen_closure
                    )
                lines = fix_max_length(lines, max_length, case)
                losses.append(total_loss.item() / temperature)

                torch.cuda.empty_cache()

                log_message = (
                    f"light_pos_id {light_pos_id:2d}, Iteration {i:3d}, Loss: {total_loss.item()/temperature:.6f}, "
                    f"mse_loss: {mse_loss.item():.6f}, perceptual_loss: {perceptual_loss.item():.6f}, "
                    f"straight_bspline_loss: {straight_bspline_loss_value.item():.6f}"
                )
                print(log_message)
                log_file.write(log_message + "\n")

                test_utils.save_image(
                    image,
                    resolution,
                    f"light_pos_{light_pos_id}/optimization_{i}.exr",
                )

        plt.figure(figsize=(10, 6))
        plt.plot(losses, label=f"light_pos_id {light_pos_id}")
        plt.xlabel("Iteration")
        plt.ylabel("Loss")
        plt.title(f"Loss Curve for light_pos_id {light_pos_id}")
        plt.legend()
        plt.savefig(f"light_pos_{light_pos_id}/loss_curve.png")

        evaluated = []

        for t in torch.linspace(1, 2, steps=16).cuda():
            evaluated.append(
                bspline.eval_quadratic_bspline_point(lines, t.repeat(lines.shape[0]))
            )

        stacked_evaluated = torch.stack(evaluated).permute(1, 0, 2)

        with open(f"light_pos_{light_pos_id}/lines.txt", "w") as f:
            f.write(str(stacked_evaluated.detach().cpu().numpy().tolist()))
