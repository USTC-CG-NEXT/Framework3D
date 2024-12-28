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
def perspective(fovy, aspect, near, far):
    f = 1.0 / np.tan(fovy / 2.0)
    m = np.zeros((4, 4))
    m[0, 0] = f / aspect
    m[1, 1] = f
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
    return image ** (1.0 / 2.2)


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

    blurred_image = TF.gaussian_blur(image, kernel_size=5, sigma=1.5)
    blurred_target = TF.gaussian_blur(target, kernel_size=5, sigma=1.5)
    mse_loss_value = torch.nn.functional.mse_loss(blurred_image, blurred_target)

    return mse_loss_value + perceptual_loss_value * 0.001


def loss_function(image, target):
    return perceptual_loss(linear_to_gamma(image), target)


def test_bspline_intersect_optimization():
    case = "lines"
    context = hd_USTC_CG_py.MeshIntersectionContext()
    if case == "bspline":
        scratch_context = hd_USTC_CG_py.BSplineScratchIntersectionContext()
        random_gen = test_utils.random_scatter_bsplines
    else:
        scratch_context = hd_USTC_CG_py.ScratchIntersectionContext()
        random_gen = test_utils.random_scatter_lines

    scratch_context.set_max_pair_buffer_ratio(12.0)

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
    print(vertices.dtype)
    assert vertices.is_contiguous()
    indices = torch.tensor([0, 1, 2, 0, 2, 3], dtype=torch.uint32).cuda()
    assert indices.is_contiguous()

    vertex_buffer_stride = 5 * 4
    resolution = [1536, 1024]

    camera_position_np = np.array([0, -3, 6], dtype=np.float32) / 1.3
    light_position_np = np.array([6, 0, 6], dtype=np.float32)

    world_to_view_matrix = look_at(
        camera_position_np, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
    )

    view_to_clip_matrix = perspective(
        np.pi / 7, resolution[0] / resolution[1], 0.1, 1000.0
    )

    width = torch.tensor([0.001], device="cuda")
    glints_roughness = torch.tensor([0.0016], device="cuda")

    import matplotlib.pyplot as plt

    max_length = 0.05

    numviews = 1

    random_gen_closure = lambda: random_gen(0.025, 15000, (0, 1), (0, 1))

    exposure = torch.tensor([50.0], device="cuda")
    exposure.requires_grad_(True)

    for view in range(numviews):
        losses = []
        lines = random_gen_closure()

        lines.requires_grad_(True)
        optimizer = torch.optim.Adam([lines], lr=0.001, betas=(0.9, 0.999), eps=1e-08)
        angle = view * (2 * np.pi / numviews)
        rotation_matrix = np.array(
            [
                [np.cos(angle), -np.sin(angle), 0],
                [np.sin(angle), np.cos(angle), 0],
                [0, 0, 1],
            ],
            dtype=np.float32,
        )
        rotated_camera_position = np.dot(rotation_matrix, camera_position_np)

        world_to_view_matrix = look_at(
            rotated_camera_position, np.array([0.0, 0, 0.0]), np.array([0.0, 0.0, 1.0])
        )

        target = renderer.prepare_target(
            "texture.png",
            context,
            vertices,
            indices,
            vertex_buffer_stride,
            resolution,
            world_to_view_matrix,
            view_to_clip_matrix,
        )

        target = target / target.max()
        test_utils.save_image(target, resolution, f"view_{view}/target.png")

        temperature = 1.0

        for i in range(400):
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
                light_position_np,
            )

            image = torch.clamp(image, 0, 1000000)

            blurred_image = torch.nn.functional.avg_pool2d(
                image, 5, stride=1, padding=2
            ).detach()
            # image = image / blurred_image.max().detach()
            # exposure = 1.0 / blurred_image.max().detach()

            image = image * exposure

            loss = loss_function(image, target) * temperature
            loss.backward()

            temperature *= 0.989

            # Mask out NaN gradients
            with torch.no_grad():
                for param in optimizer.param_groups[0]["params"]:
                    if param.grad is not None:
                        nan_mask = torch.isnan(param.grad)
                        merged_nan_mask = nan_mask.any(dim=1).any(dim=1)

                        print(f"Number of NaNs: {merged_nan_mask.sum()}")
                        param.grad[merged_nan_mask] = torch.empty_like(
                            param.grad[merged_nan_mask]
                        ).uniform_(-0.1, 0.1)

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

            with torch.no_grad():
                lines.clamp_(0, 1)

            # if i % 3 == 0 and i < 90:
            #     with torch.no_grad():
            #         lines[low_contribution_mask] = random_gen_closure()[
            #             low_contribution_mask
            #         ]
            # if i < 80:
            #     with torch.no_grad():
            #         lines[merged_nan_mask] = random_gen_closure()[merged_nan_mask]
            losses.append(loss.item())

            print(
                f"View {view}, Iteration {i}, Loss: {loss.item()/temperature}, current exposure: {exposure.item()}"
            )

            torch.cuda.empty_cache()

            test_utils.save_image(
                linear_to_gamma(image), resolution, f"view_{view}/optimization_{i}.png"
            )

        with open(f"view_{view}/lines.txt", "w") as f:
            f.write(str(torch.nan_to_num(lines, nan=-1.0)[:, :, :2].tolist()))

        # Plot the loss curve
        plt.plot(losses)
        plt.xlabel("Iteration")
        plt.ylabel("Loss")
        plt.title(f"Loss Curve for View {view}")
        plt.savefig(f"view_{view}/loss_curve.png")
