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


# import numpy as np
# import torch

# import drjit as dr
from data.data_structures import CameraInfo


def exec_node(a, b):
    # print("a ", type(a))
    # print("b ", type(b))

    # c =TorchTensor(a,device='cuda')
    # d = c+b

    camera = CameraInfo()
    a = dict()

    return a + b
