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
    print(result)  # tensor([[0.0019914089, 0.0185666792]], device='cuda:0')


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
            [2.6758241002e-03, 1.7571991310e-02],
            [2.3046473507e-03, 1.7311304808e-02],
            [1.9475375302e-03, 1.6737971455e-02],
            [1.2018895941e-03, 1.1761784554e-02],
            [6.7839352414e-04, 7.4517717585e-03],
            [2.9678267310e-04, 3.6749383435e-03],
            [2.7504982427e-05, 4.5287329704e-04],
            [0.0000000000e00, 0.0000000000e00],
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
