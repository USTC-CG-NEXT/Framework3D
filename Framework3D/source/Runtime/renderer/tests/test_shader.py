import glints.shaderAB
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

    result = glints.shaderAB.ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )
    print(result)  # (0.000877353, 0.00714865)


def test_shader_expanded():
    lines = torch.tensor(
        [
            [[0.0, 0.1], [1.0, 0.0]],
            [[0.0, 0.2], [1.0, 0.0]],
            [[0.0, 0.3], [1.0, 0.0]],
            [[0.0, 0.4], [1.0, 0.0]],
            [[0.0, 0.5], [1.0, 0.0]],
            [[0.0, 0.6], [1.0, 0.0]],
            [[0.0, 0.7], [1.0, 0.0]],
            [[0.0, 0.8], [1.0, 0.0]],
        ],
        device="cuda",
    )
    patches = torch.tensor(
        [
            [[0.28, 0.24], [0.65, 0.35], [0.76, -0.27], [0.25, -0.25]],
            [[0.27, 0.23], [0.64, 0.34], [0.75, -0.28], [0.24, -0.26]],
            [[0.26, 0.22], [0.63, 0.33], [0.74, -0.29], [0.23, -0.27]],
            [[0.25, 0.21], [0.62, 0.32], [0.73, -0.30], [0.22, -0.28]],
            [[0.24, 0.20], [0.61, 0.31], [0.72, -0.31], [0.21, -0.29]],
            [[0.23, 0.19], [0.60, 0.30], [0.71, -0.32], [0.20, -0.30]],
            [[0.22, 0.18], [0.59, 0.29], [0.70, -0.33], [0.19, -0.31]],
            [[0.21, 0.17], [0.58, 0.28], [0.69, -0.34], [0.18, -0.32]],
        ],
        device="cuda",
    )

    cam_positions = torch.tensor(
        [
            [1.0, 1.0, 1.0],
            [1.1, 1.0, 1.0],
            [1.2, 1.0, 1.0],
            [1.3, 1.0, 1.0],
            [1.4, 1.0, 1.0],
            [1.5, 1.0, 1.0],
            [1.6, 1.0, 1.0],
            [1.7, 1.0, 1.0],
        ],
        device="cuda",
    )
    light_positions = torch.tensor(
        [
            [-1.1, 1.0, 1.0],
            [-1.2, 1.0, 1.0],
            [-1.3, 1.0, 1.0],
            [-1.4, 1.0, 1.0],
            [-1.5, 1.0, 1.0],
            [-1.6, 1.0, 1.0],
            [-1.7, 1.0, 1.0],
            [-1.8, 1.0, 1.0],
        ],
        device="cuda",
    )

    lines = lines.expand(patches.shape[0], -1, -1)

    glints_roughness = torch.tensor([0.2], device="cuda")
    width = torch.tensor([0.02], device="cuda")

    result = glints.shaderAB.ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )
    expected_result = torch.tensor(
        [
            [0.00123411, 0.0070288],
            [0.00106927, 0.00692452],
            [0.000933587, 0.0067777],
            [0.000581921, 0.00470471],
            [0.000334815, 0.00298071],
            [0.000153506, 0.00146997],
            [1.29429e-05, 9.82016e-05],
            [0.0, 0.0],
        ],
        device="cuda:0",
    )
    assert torch.allclose(result, expected_result, atol=1e-6)

    print(result)


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
