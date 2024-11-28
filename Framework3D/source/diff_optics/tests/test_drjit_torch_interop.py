import drjit as dr
import torch

def test_dr_wrap():
    a = torch.tensor([1.0, 2.0, 3.0], device="cuda")
    c = dr.wrap("torch")