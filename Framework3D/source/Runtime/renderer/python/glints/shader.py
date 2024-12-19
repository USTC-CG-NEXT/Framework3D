import torch


class GlintsTracingParams:
    def __init__(self, cam_position, light_position, width, glints_roughness):
        self.cam_position = torch.tensor(cam_position, dtype=torch.float32)
        self.light_position = torch.tensor(light_position, dtype=torch.float32)
        self.width = torch.tensor(width, dtype=torch.float32)
        self.glints_roughness = torch.tensor(glints_roughness, dtype=torch.float32)


def cross_2d(a, b):
    return a[:, 0] * b[:, 1] - a[:, 1] * b[:, 0]


def signed_area(line, point):
    line_direction = torch.nn.functional.normalize(line[:, 1, :] - line[:, 0, :])
    midpoint = (line[:, 0, :] + line[:, 1, :]) / 2.0
    return cross_2d(point - midpoint, -line_direction)


def integral_triangle_area(p0, p1, p2, t, axis):
    dot_p1_p0 = torch.sum((p1 - p0) * axis, dim=1)
    dot_p2_p0 = torch.sum((p2 - p0) * axis, dim=1)
    dot_p1_p2 = torch.sum((p1 - p2) * axis, dim=1)
    dot_p0_p2 = torch.sum((p0 - p2) * axis, dim=1)

    condition1 = (t >= 0) & (t <= dot_p1_p0)
    condition2 = (t > dot_p1_p0) & (t <= dot_p2_p0)
    condition3 = t > dot_p2_p0

    area1 = torch.abs(cross_2d(t[:, None] / dot_p2_p0[:, None] * (p2 - p0), t[:, None] / dot_p1_p0[:, None] * (p1 - p0))) / 2.0
    area2 = torch.abs(cross_2d((p2 - p0), (p1 - p0))) / 2.0 - torch.abs(cross_2d((p1 - p2) * (dot_p2_p0[:, None] - t[:, None]) / dot_p1_p2[:, None], (p0 - p2) * (dot_p2_p0[:, None] - t[:, None]) / dot_p0_p2[:, None])) / 2.0
    area3 = torch.abs(cross_2d((p2 - p0), (p1 - p0))) / 2.0

    result = torch.where(condition1, area1, torch.where(condition2, area2, torch.where(condition3, area3, torch.tensor(0.0, device=p0.device))))
    return result

def intersect_triangle_area(p0, p1, p2, line, width):
    width_half = width / 2.0

    line_pos = (line[:, 0, :] + line[:, 1, :]) / 2.0
    line_dir = torch.nn.functional.normalize(line[:, 1, :] - line[:, 0, :], dim=1)

    vertical_dir = torch.stack((line_dir[:, 1], -line_dir[:, 0]), dim=1)

    p0_tmp = torch.where(
        (torch.sum((p0 - p1) * vertical_dir, dim=1) >= 0) & (torch.sum((p2 - p1) * vertical_dir, dim=1) >= 0),
        p1,
        p0
    )

    p1_tmp = torch.where(
        (torch.sum((p0 - p1) * vertical_dir, dim=1) >= 0) & (torch.sum((p2 - p1) * vertical_dir, dim=1) >= 0),
        p0,
        p1
    )

    p0_t = p0_tmp
    p1_t = p1_tmp
    p2_t = p2

    p0_tmp = torch.where(
        (torch.sum((p0_t - p2_t) * vertical_dir, dim=1) >= 0) & (torch.sum((p1_t - p2_t) * vertical_dir, dim=1) >= 0),
        p2_t,
        p0_t
    )

    p2_tmp = torch.where(
        (torch.sum((p0_t - p2_t) * vertical_dir, dim=1) >= 0) & (torch.sum((p1_t - p2_t) * vertical_dir, dim=1) >= 0),
        p0_t,
        p2_t
    )

    x_to_vertical_dir1 = torch.sum((p1_tmp - p0_tmp) * vertical_dir, dim=1)
    x_to_vertical_dir2 = torch.sum((p2_tmp - p0_tmp) * vertical_dir, dim=1)

    p1_tmptmp = p1_tmp
    p2_tmptmp = p2_tmp
    p1_tmp = torch.where(x_to_vertical_dir1 >= x_to_vertical_dir2, p2_tmptmp, p1_tmptmp)
    p2_tmp = torch.where(x_to_vertical_dir1 >= x_to_vertical_dir2, p1_tmptmp, p2_tmptmp)

    t1 = torch.sum((line_pos - p0_tmp) * vertical_dir, dim=1) - width_half
    t2 = torch.sum((line_pos - p0_tmp) * vertical_dir, dim=1) + width_half

    return integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t2, vertical_dir) - integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t1, vertical_dir)

def intersect_area(line, patch, width):
    p0 = patch[:, 0, :]
    p1 = patch[:, 1, :]
    p2 = patch[:, 2, :]
    p3 = patch[:, 3, :]

    a = intersect_triangle_area(p0, p1, p2, line, width)
    b = intersect_triangle_area(p2, p3, p0, line, width)

    return a + b





# line shape: [n, 2, 2]
# patch shape: [n, 4, 2]
# cam_positions shape: [n, 3]
# light_positions shape: [n, 3]
# glints_roughness shape: [1]
# width shape: [1]
def ShadeLineElement(
    lines, patches, cam_positions, light_positions, glints_roughness, width
):
    assert lines.shape[0] == patches.shape[0]

    camera_pos_uv = cam_positions.cuda()
    light_pos_uv = light_positions.cuda()

    p0 = patches[:, 0, :].cuda()
    p1 = patches[:, 1, :].cuda()
    p2 = patches[:, 2, :].cuda()
    p3 = patches[:, 3, :].cuda()

    center = (p0 + p1 + p2 + p3) / 4.0

    p = torch.stack(
        (
            center[:, 0],
            center[:, 1],
            torch.zeros(center.shape[0], device=center.device),
        ),
        dim=1,
    )
    camera_dir = torch.nn.functional.normalize(camera_pos_uv - p)
    light_dir = torch.nn.functional.normalize(light_pos_uv - p)

    cam_dir_2D = camera_dir[:, :2]
    light_dir_2D = light_dir[:, :2]

    line_direction = torch.nn.functional.normalize(lines[:, 1, :] - lines[:, 0, :])

    local_cam_dir = torch.stack(
        (
            cross_2d(cam_dir_2D, line_direction),
            torch.sum(cam_dir_2D * line_direction, dim=1),
            camera_dir[:, 2],
        ),
        dim=1,
    )

    local_light_dir = torch.stack(
        (
            cross_2d(light_dir_2D, line_direction),
            torch.sum(light_dir_2D * line_direction, dim=1),
            light_dir[:, 2],
        ),
        dim=1,
    )

    half_vec = torch.nn.functional.normalize(local_cam_dir + local_light_dir)

    a0 = signed_area(lines, p0)
    a1 = signed_area(lines, p1)
    a2 = signed_area(lines, p2)
    a3 = signed_area(lines, p3)


    minimum = torch.min(torch.min(torch.min(a0, a1), a2), a3)
    maximum = torch.max(torch.max(torch.max(a0, a1), a2), a3)

    area = intersect_area(lines, patches, 2.0 * width)

    print(area)

    pass
