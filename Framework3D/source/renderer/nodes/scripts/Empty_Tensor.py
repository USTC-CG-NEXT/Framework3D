def declare_node():
    return [{}, {"tensor": "TorchTensor"}]


def wrap_exec(list):
    return exec_node(*list)


import torch
torch.set_default_device('cuda')

def exec_node():
    return torch.empty(1, device="cuda")
