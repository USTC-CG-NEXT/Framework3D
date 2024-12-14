import drjit as dr
import torch


def test_dr_wrap():
    a = torch.tensor([1.0, 2.0, 3.0], device="cuda", dtype=torch.float32)
    c = dr.cuda.ad.TensorXf(a).array
    print(c)