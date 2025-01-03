# A scratch field is a torch tensor, shaped [n,n,m,2]

import torch


class ScratchField:
    def __init__(self, n, m):
        self.n = n
        self.m = m

        random_theta = (
            torch.rand((n, n, m), dtype=torch.float32, device="cuda") * 2 * 3.14159
        )
        self.field = torch.stack(
            [torch.cos(random_theta), torch.sin(random_theta)], dim=3
        )

        self.field[:, :, :, 1] += 2

        self.field.requires_grad = True

    def calc_divergence(self):

        divergence = torch.zeros(
            (self.n, self.n, self.m), dtype=torch.float32, device="cuda"
        )

        for i in range(self.m):
            field_x = self.field[:, :, i, 0]
            field_y = self.field[:, :, i, 1]

            dx = (field_x[2:, 1:-1] - field_x[:-2, 1:-1]) / 2.0
            dy = (field_y[1:-1, 2:] - field_y[1:-1, :-2]) / 2.0

            divergence[1:-1, 1:-1, i] = dx + dy

        assert torch.isnan(divergence).sum() == 0

        return divergence

    def calc_smoothness(self):
        # Calculate the smoothness of the field
        smoothness = torch.zeros(
            (self.n, self.n, self.m), dtype=torch.float32, device="cuda"
        )

        for i in range(self.m):
            field_x = self.field[:, :, i, 0]
            field_y = self.field[:, :, i, 1]
            # this is a vector field, calculate the smoothness of the field
            # and this is going to be the loss later. We want the field to be smooth

            dx = field_x[2:, 1:-1] - field_x[1:-1, 1:-1]
            dy = field_y[1:-1, 2:] - field_y[1:-1, 1:-1]
            smoothness[1:-1, 1:-1, i] = dx**2 + dy**2

        return smoothness

    def same_density_projection(self):
        lengths = torch.norm(self.field, dim=3)
        self.field /= lengths.unsqueeze(3)

    def sample(self, uv):
        """
        uv: torch.tensor of shape [count,2], where uv[:,0] is u and uv[:,1] is v, both in [0,1]
        """

        coord = uv * (self.n - 1)
        u0 = torch.floor(coord[:, 0]).long()
        v0 = torch.floor(coord[:, 1]).long()
        u1 = u0 + 1
        v1 = v0 + 1

        u0 = torch.clamp(u0, 0, self.n - 1)
        v0 = torch.clamp(v0, 0, self.n - 1)
        u1 = torch.clamp(u1, 0, self.n - 1)
        v1 = torch.clamp(v1, 0, self.n - 1)

        u = coord[:, 0] - u0.float()
        v = coord[:, 1] - v0.float()

        u = u.unsqueeze(1).unsqueeze(2)
        v = v.unsqueeze(1).unsqueeze(2)

        f00 = self.field[u0, v0]
        f01 = self.field[u0, v1]
        f10 = self.field[u1, v0]
        f11 = self.field[u1, v1]

        sampled = (
            (1 - u) * (1 - v) * f00
            + (1 - u) * v * f01
            + u * (1 - v) * f10
            + u * v * f11
        )  # shape [count, m, 2]

        sampled = sampled.reshape(-1, 2)  # shape [count*m, 2]

        line_weight = torch.norm(sampled, dim=1)

        line_direction = sampled / line_weight.unsqueeze(1)
        line_center = uv.repeat(1, self.m).reshape(-1, 2)

        lines_random_z = torch.zeros(line_center.shape[0], device="cuda")

        lines_begin = line_center - line_direction * 0.0001  # shape [count, 2]
        lines_begin = torch.cat([lines_begin, lines_random_z.unsqueeze(1)], dim=1)

        lines_end = line_center + line_direction * 0.0001
        lines_end = torch.cat([lines_end, lines_random_z.unsqueeze(1)], dim=1)

        lines = torch.cat([lines_begin, lines_end], dim=1)
        lines = lines.reshape(-1, 2, 3).contiguous()

        return lines, line_weight


def render_scratch_field(renderer, resolution, field):

    _, _, _, uv = renderer.preliminary_render(resolution)

    lines, line_weight = field.sample(uv)

    image, low_contribution_mask = renderer.render(
        resolution, lines, force_single_line=True, line_weight=line_weight
    )

    return image
