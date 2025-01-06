# A scratch field is a torch tensor, shaped [n,n,m,2]

import torch


class ScratchField:
    def __init__(self, n, m):
        self.n = n
        self.m = m

        random_theta = (
            torch.rand((n, n, m), dtype=torch.float32, device="cuda") - 0.5
        ) * 0.4 + 0.5 * torch.pi

        self.field = (
            torch.stack([torch.cos(random_theta), torch.sin(random_theta)], dim=3) * 0.5
        )

        self.field.requires_grad = True

    def calc_divergence_smoothness(self):

        divergence = torch.zeros(
            (self.n, self.n, self.m), dtype=torch.float32, device="cuda"
        )

        smoothness = torch.zeros(
            (self.n, self.n, self.m), dtype=torch.float32, device="cuda"
        )

        for i in range(self.m):

            field = self.field[:, :, i]

            field_left = field[1:-1, :-2]
            same_directioned_field_left = field_left

            field_right = field[1:-1, 2:]
            sign_right = torch.sign(torch.sum(field_left * field_right, dim=2))
            same_directioned_field_right = field_right * sign_right.unsqueeze(2)

            field_up = field[:-2, 1:-1]
            same_directioned_field_up = field_up

            field_down = field[2:, 1:-1]
            sign_down = torch.sign(torch.sum(field_up * field_down, dim=2))
            same_directioned_field_down = field_down * sign_down.unsqueeze(2)

            dx = (
                same_directioned_field_right[:, :, 0]
                - same_directioned_field_left[:, :, 0]
            )
            dy = (
                same_directioned_field_up[:, :, 1]
                - same_directioned_field_down[:, :, 1]
            )

            divergence[1:-1, 1:-1, i] = torch.abs(dx + dy) * self.n

            smoothness[1:-1, 1:-1, i] = dx**2 + dy**2

        assert torch.isnan(divergence).sum() == 0

        return divergence, smoothness

    def same_density_projection(self):
        lengths = torch.norm(self.field, dim=3)
        self.field /= lengths.unsqueeze(3)

    def fix_direction(self):
        with torch.no_grad():
            sign_x = torch.sign(self.field[:, :, :, 1])
            self.field *= sign_x.unsqueeze(3)

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

        sampled_mask = torch.zeros_like(self.field, dtype=torch.bool)
        sampled_mask[u0, v0] = True
        sampled_mask[u1, v0] = True
        sampled_mask[u0, v1] = True
        sampled_mask[u1, v1] = True

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

        return lines, line_weight, sampled_mask


def render_scratch_field(renderer, resolution, field):

    _, _, _, uv = renderer.preliminary_render(resolution)

    lines, line_weight, sampled_mask = field.sample(uv)

    image, low_contribution_mask = renderer.render(
        resolution, lines, force_single_line=True, line_weight=line_weight
    )

    return image, sampled_mask
