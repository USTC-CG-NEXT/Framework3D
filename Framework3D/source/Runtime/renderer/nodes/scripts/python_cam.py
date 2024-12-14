# script.py


def declare_node():
    return [
        {
            "idx": "Int",
            "R": "NumpyArray",
            "T": "NumpyArray",
            "fovy": "NumpyArray",
            "fovx": "NumpyArray",
            "image": "TorchTensor",
            "world_view_transform": "TorchTensor",
            "projection_matrix": "TorchTensor",
            "full_proj_transform": "TorchTensor",
            "camera_center": "TorchTensor",
            "time": "Float",
            "rays": "TorchTensor",
            "event_image": "TorchTensor",
        },
        {"Camera": "PyObj"},
    ]


def wrap_exec(list):
    return exec_node(*list)


from data.data_structures import CameraInfo
import numpy as np
import torch

torch.set_default_device("cuda")


def exec_node(
    idx: int,
    R: np.ndarray,
    T: np.ndarray,
    fovy: np.ndarray,
    fovx: np.ndarray,
    image: torch.Tensor,
    world_view_transform: torch.Tensor,
    projection_matrix: torch.Tensor,
    full_proj_transform: torch.Tensor,
    camera_center: torch.Tensor,
    time: float,
    rays: torch.Tensor,
    event_image: torch.Tensor,
):

    camera = CameraInfo(
        idx,
        R,
        T,
        fovy,
        fovx,
        image,
        world_view_transform,
        projection_matrix,
        full_proj_transform,
        camera_center,
        time,
        rays,
        event_image,
        [],
    )

    return camera
