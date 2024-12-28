import glints.shader
import glints.test_utils as test_utils
import torch


def test_shader():
    lines = torch.tensor([[[0.0, 0.0], [1.0, 0.0]]], device="cuda")
    patches = torch.tensor(
        [[[0.28, 0.24], [0.65, 0.35], [0.76, -0.27], [0.25, -0.25]]], device="cuda"
    )

    cam_positions = torch.tensor([1.0, 1.0, 1.0], device="cuda")
    light_positions = torch.tensor([-1.8, 1.0, 1.0], device="cuda")

    lines = lines.expand(patches.shape[0], -1, -1)

    glints_roughness = torch.tensor([0.2], device="cuda")
    width = torch.tensor([0.02], device="cuda")

    result = glints.shader.ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )
    print(result)  # tensor([[0.0019914089, 0.0185666792]], device='cuda:0')


# def test_shader_expanded():
#     lines = torch.tensor(
#         [
#             [[0.0, 0.0], [1.0, 0.0]],
#             [[0.0, 0.0], [1.0, 0.0]],
#             [[0.0, 0.0], [1.0, 0.0]],
#             [[0.0, 0.0], [1.0, 0.0]],
#         ],
#         device="cuda",
#     )
#     patches = torch.tensor(
#         [
#             [[0.25, 0.25], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#         ],
#         device="cuda",
#     )

#     cam_positions = torch.tensor(
#         [[1.0, 1.0, 1.0], [1.3, 1.0, 1.0], [1.3, 1.0, 1.0], [1.3, 1.0, 1.0]],
#         device="cuda",
#     )
#     light_positions = torch.tensor(
#         [[-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0]],
#         device="cuda",
#     )

#     lines = lines.expand(patches.shape[0], -1, -1)

#     glints_roughness = torch.tensor([0.2], device="cuda")
#     width = torch.tensor([0.02], device="cuda")

#     result = glints.shader.ShadeLineElement(
#         lines, patches, cam_positions, light_positions, glints_roughness, width
#     )
#     print(result)


# def test_bspline_shader_expanded():
#     ctr_points = torch.tensor(
#         [
#             [[0.0, -1.0], [1.0, 1.0], [2.0, -1.0]],
#             [[0.0, -1.0], [1.0, 1.0], [2.0, -1.0]],
#             [[0.0, -1.0], [1.0, 1.0], [2.0, -1.0]],
#             [[0.0, -1.0], [1.0, 1.0], [2.0, -1.0]],
#         ],
#         device="cuda",
#     )
#     patches = torch.tensor(
#         [
#             [[0.25, 0.25], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#             [[0.25, 0.15], [0.65, 0.35], [0.76, -0.25], [0.25, -0.25]],
#         ],
#         device="cuda",
#     )

#     cam_positions = torch.tensor(
#         [[1.0, 1.0, 1.0], [1.3, 1.0, 1.0], [1.3, 1.0, 1.0], [1.3, 1.0, 1.0]],
#         device="cuda",
#     )
#     light_positions = torch.tensor(
#         [[-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0], [-1.8, 1.0, 1.0]],
#         device="cuda",
#     )

#     ctr_points = ctr_points.expand(patches.shape[0], -1, -1)

#     glints_roughness = torch.tensor([0.2], device="cuda")
#     width = torch.tensor([0.02], device="cuda")

#     result = glints.shader.ShadeBSplineElements(
#         ctr_points, patches, cam_positions, light_positions, glints_roughness, width
#     )
#     print(result)
