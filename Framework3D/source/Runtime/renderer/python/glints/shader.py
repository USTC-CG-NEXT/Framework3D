import torch


class GlintsTracingParams:
    def __init__(self, cam_position, light_position, width, glints_roughness):
        self.cam_position = torch.tensor(cam_position, dtype=torch.float32)
        self.light_position = torch.tensor(light_position, dtype=torch.float32)
        self.width = torch.tensor(width, dtype=torch.float32)
        self.glints_roughness = torch.tensor(glints_roughness, dtype=torch.float32)


# cam_position shape: [3]
# worldToClip shape: [4, 4]
# patch shape: [n, 4, 2]
# ret shape: [n, 3]
def world_space_to_patch_space(patches, worldToClip, cam_position):

    pass


# line shape: [n, 2, 2]
# patch shape: [n, 4, 2]
def ShadeLineElement(lines, patches, glints_params):
    assert lines.shape[0] == patches.shape[0]

    pass
