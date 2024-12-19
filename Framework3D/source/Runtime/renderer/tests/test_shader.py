import glints.shader
import glints.test_utils as test_utils
import torch


def test_shader():
    lines = torch.tensor([[[0.0, 0.0], [1.0, 0.0]]], device="cuda")
    patches = torch.tensor(
        [[[0.25, 0.25], [0.65, 0.25], [0.76, -0.25], [0.25, -0.25]]], device="cuda"
    )

    cam_positions = torch.tensor([1.0, 1.0, 1.0], device="cuda")
    light_positions = torch.tensor([-1.0, 1.0, 1.0], device="cuda")

    lines = lines.expand(patches.shape[0], -1, -1)

    glints_roughness = torch.tensor([0.1], device="cuda")
    width = torch.tensor([0.2], device="cuda")

    glints.shader.ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )
    