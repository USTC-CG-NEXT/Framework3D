# script.py


def declare_node():
    return [["a", "b"], ["c"]]


def wrap_exec(list):
    return exec_node(*list)


# import numpy as np
# import torch

# import drjit as dr
from data.data_structures import CameraInfo

def exec_node(a, b):
    # print("a ", type(a))
    # print("b ", type(b))
    
    # c =torch.tensor(a,device='cuda')
    # d = c+b

    camera = CameraInfo()
    a = dict()
    

    return a+b
