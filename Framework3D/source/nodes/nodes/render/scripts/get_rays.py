import torch
import torch
import torch.nn.functional as F
from kornia import create_meshgrid

from data.data_structures import CameraInfo


def pix2ndc(v, S):
    return (v * 2.0 + 1.0) / S - 1.0


def declare_node():
    return [
        {
            "camera": "PyObj",
            "h": "Int",
            "w": "Int",
        },
        {"rays": "TorchTensor"},
    ]


def exec_node(camera: CameraInfo, h: int, w: int):

    pinv = camera.projection_matrix.reshape(4, 4).T.inverse()
    c2w = camera.world_view_transform.reshape(4, 4).T.inverse()
    pixgrid = create_meshgrid(h, w, normalized_coordinates=False)[0]

    xidx = pixgrid[:, :, 0]
    yidx = pixgrid[:, :, 1]

    ndcy, ndcx = pix2ndc(yidx, h).unsqueeze(-1), pix2ndc(xidx, w).unsqueeze(-1)
    ndccamera = torch.cat(
        (ndcx, ndcy, torch.ones_like(ndcx), torch.ones_like(ndcy)), dim=2
    ).to(pinv.device)

    projected = ndccamera @ pinv.T
    local_dir = projected / projected[:, :, 3:]

    direction = local_dir[:, :, :3] @ c2w[:3, :3].T
    rays_d = F.normalize(direction, p=2.0, dim=-1)

    rayo = camera.camera_center.expand(rays_d.shape).permute(2, 0, 1).unsqueeze(0)
    rayd = rays_d.permute(2, 0, 1).unsqueeze(0)

    
    rays = torch.cat((rayo, rayd), dim=1)
    return rays


def wrap_exec(list):
    return exec_node(*list)
