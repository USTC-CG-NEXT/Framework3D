import glints.shader
import glints.test_utils as test_utils
import torch


def test_shader():
    lines = torch.tensor([[[0.0, 0.0], [1.0, 1.0]]], device="cuda")
    patches = test_utils.create_patches(1024, 2.0 / 1024)  # [1024*1024,4,2]
    
    lines = lines.expand(patches.shape[0], -1, -1)
    
    glints_params = glints.shader.GlintsTracingParams(
        [0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 0.5, 0.5
    )

    glints.shader.ShadeLineElement(lines, patches, glints_params)
