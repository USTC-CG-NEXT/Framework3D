def declare_node():
    return [{}, {"tensor": "TorchTensor"}]


def wrap_exec(list):
    return exec_node(*list)


import torch


def exec_node():
    return torch.empty(1, device="cuda")
