# A scratch field is a torch tensor, shaped [n,n,m,2]

import torch


class ScratchField:
    def __init__(self, n, m):
        self.n = n
        self.m = m
        self.field = torch.zeros((n, n, m, 2), dtype=torch.float32, device="cuda")

    def calc_divergence(self):
        pass

    def sample(self, uv):
        pass


def render_scratch_field(renderer, field):
    pass
