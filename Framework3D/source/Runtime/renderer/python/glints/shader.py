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

    area1 = (
        torch.abs(
            cross_2d(
                t[:, None] / dot_p2_p0[:, None] * (p2 - p0),
                t[:, None] / dot_p1_p0[:, None] * (p1 - p0),
            )
        )
        / 2.0
    )
    area2 = (
        torch.abs(cross_2d((p2 - p0), (p1 - p0))) / 2.0
        - torch.abs(
            cross_2d(
                (p1 - p2) * (dot_p2_p0[:, None] - t[:, None]) / dot_p1_p2[:, None],
                (p0 - p2) * (dot_p2_p0[:, None] - t[:, None]) / dot_p0_p2[:, None],
            )
        )
        / 2.0
    )
    area3 = torch.abs(cross_2d((p2 - p0), (p1 - p0))) / 2.0

    result = torch.where(
        condition1,
        area1,
        torch.where(
            condition2,
            area2,
            torch.where(condition3, area3, torch.tensor(0.0, device=p0.device)),
        ),
    )
    return result


def intersect_triangle_area(p0, p1, p2, line, width):
    width_half = width / 2.0

    line_pos = (line[:, 0, :] + line[:, 1, :]) / 2.0
    line_dir = torch.nn.functional.normalize(line[:, 1, :] - line[:, 0, :], dim=1)

    vertical_dir = torch.stack((line_dir[:, 1], -line_dir[:, 0]), dim=1)

    p0_tmp = torch.where(
        (torch.sum((p0 - p1) * vertical_dir, dim=1).unsqueeze(1) >= 0)
        & (torch.sum((p2 - p1) * vertical_dir, dim=1).unsqueeze(1) >= 0),
        p1,
        p0,
    )

    p1_tmp = torch.where(
        (torch.sum((p0 - p1) * vertical_dir, dim=1).unsqueeze(1) >= 0)
        & (torch.sum((p2 - p1) * vertical_dir, dim=1).unsqueeze(1) >= 0),
        p0,
        p1,
    )

    p0_t = p0_tmp
    p1_t = p1_tmp
    p2_t = p2

    p0_tmp = torch.where(
        (torch.sum((p0_t - p2_t) * vertical_dir, dim=1).unsqueeze(1) >= 0)
        & (torch.sum((p1_t - p2_t) * vertical_dir, dim=1).unsqueeze(1) >= 0),
        p2_t,
        p0_t,
    )

    p2_tmp = torch.where(
        (torch.sum((p0_t - p2_t) * vertical_dir, dim=1).unsqueeze(1) >= 0)
        & (torch.sum((p1_t - p2_t) * vertical_dir, dim=1).unsqueeze(1) >= 0),
        p0_t,
        p2_t,
    )

    x_to_vertical_dir1 = torch.sum((p1_tmp - p0_tmp) * vertical_dir, dim=1)
    x_to_vertical_dir2 = torch.sum((p2_tmp - p0_tmp) * vertical_dir, dim=1)

    p1_tmptmp = p1_tmp
    p2_tmptmp = p2_tmp
    p1_tmp = torch.where(
        (x_to_vertical_dir1 >= x_to_vertical_dir2).unsqueeze(1), p2_tmptmp, p1_tmptmp
    )
    p2_tmp = torch.where(
        (x_to_vertical_dir1 >= x_to_vertical_dir2).unsqueeze(1), p1_tmptmp, p2_tmptmp
    )

    t1 = torch.sum((line_pos - p0_tmp) * vertical_dir, dim=1) - width_half
    t2 = torch.sum((line_pos - p0_tmp) * vertical_dir, dim=1) + width_half

    return integral_triangle_area(
        p0_tmp, p1_tmp, p2_tmp, t2, vertical_dir
    ) - integral_triangle_area(p0_tmp, p1_tmp, p2_tmp, t1, vertical_dir)


def intersect_area(line, patch, width):
    p0 = patch[:, 0, :]
    p1 = patch[:, 1, :]
    p2 = patch[:, 2, :]
    p3 = patch[:, 3, :]

    a = intersect_triangle_area(p0, p1, p2, line, width)
    b = intersect_triangle_area(p2, p3, p0, line, width)

    return a + b


def calc_power_series(tensor):
    powers = [tensor]
    for i in range(2, 7):
        powers.append(powers[-1] * tensor)
    return powers


def power(tensor_powers, n):
    return tensor_powers[n - 1]


def sumpart(lower, upper, y, width_powers, halfX_powers, halfZ_powers, r_powers):
    y_powers = calc_power_series(y)

    log_val_u = torch.log(upper - y)
    log_val_l = torch.log(lower - y)

    halfX = power(halfX_powers, 1)
    halfZ = power(halfZ_powers, 1)
    width = power(width_powers, 1)
    r = power(r_powers, 1)

    halfX2 = power(halfX_powers, 2)
    halfZ2 = power(halfZ_powers, 2)
    halfX4 = power(halfX_powers, 4)
    halfZ4 = power(halfZ_powers, 4)
    r2 = power(r_powers, 2)
    r4 = power(r_powers, 4)
    width2 = power(width_powers, 2)
    width3 = power(width_powers, 3)

    term1 = (halfZ - halfZ2 * halfZ * r2) ** 2 + halfX2 * (-1 + halfZ4 * r4)
    term2 = (
        -4
        * halfX
        * halfZ
        * (-2 + (3 * halfX2 + halfZ2) * r2 + halfZ2 * (halfX2 + halfZ2) * r4)
    )
    term3 = 4 * (
        -7 * halfX2
        + 7 * halfZ2
        + 2 * (3 * halfX4 - 2 * halfX2 * halfZ2 - 4 * halfZ4) * r2
        + halfZ2 * (halfX2 + halfZ2) * (2 * halfX2 + halfZ2) * r4
    )
    term4 = 64 * halfX * halfZ * (-1 + (halfX2 + halfZ2) * r2)

    a = -(
        (
            term1 * width3
            + term2 * width2 * y
            + term3 * width * y_powers[1]
            + term4 * y_powers[2]
        )
        * (log_val_u - log_val_l)
    )

    b = (
        halfX * halfZ * r2 * width3
        + 2 * (1 + (-2 * halfX2 + halfZ2) * r2) * width2 * y
        - 12 * halfX * halfZ * r2 * width * y_powers[1]
        - 8 * (-1 + halfZ2 * r2) * y_powers[2]
    )

    return a / b


def calc_res(value, width_powers, halfX_powers, halfZ_powers, r_powers):
    x_powers = calc_power_series(value)
    halfX = power(halfX_powers, 1)
    halfZ = power(halfZ_powers, 1)
    width = power(width_powers, 1)
    a = (
        4
        * (
            halfX
            * halfZ
            * power(r_powers, 2)
            * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
            * (
                -2
                + (3 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 2)
                + power(halfZ_powers, 2)
                * (power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 4)
            )
            * power(width_powers, 5)
            - 2
            * (
                torch.pow(-1.0 + power(halfZ_powers, 2) * power(r_powers, 2), 3)
                * (1 + power(halfZ_powers, 2) * power(r_powers, 2))
                + 4
                * power(halfX_powers, 4)
                * power(halfZ_powers, 2)
                * power(r_powers, 6)
                * (3 + power(halfZ_powers, 2) * power(r_powers, 2))
                + power(halfX_powers, 2)
                * power(r_powers, 2)
                * (
                    2
                    - 11 * power(halfZ_powers, 2) * power(r_powers, 2)
                    + 4 * power(halfZ_powers, 4) * power(r_powers, 4)
                    + 5 * power(halfZ_powers, 6) * power(r_powers, 6)
                )
            )
            * power(width_powers, 4)
            * x_powers[0]
            + 4
            * halfX
            * halfZ
            * power(r_powers, 2)
            * (
                6
                + (-19 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 2)
                + 2
                * (
                    6 * power(halfX_powers, 4)
                    - power(halfX_powers, 2) * power(halfZ_powers, 2)
                    - 4 * power(halfZ_powers, 4)
                )
                * power(r_powers, 4)
                + power(halfZ_powers, 2)
                * (power(halfX_powers, 2) + power(halfZ_powers, 2))
                * (4 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 6)
            )
            * power(width_powers, 3)
            * power(x_powers, 2)
            + 8
            * (1 + power(halfZ_powers, 2) * power(r_powers, 2))
            * (
                -1
                + (2 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 2)
            )
            * (
                -2
                + (3 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 2)
                + power(halfZ_powers, 2)
                * (power(halfX_powers, 2) + power(halfZ_powers, 2))
                * power(r_powers, 4)
            )
            * power(width_powers, 2)
            * power(x_powers, 3)
            - 64
            * halfX
            * halfZ
            * power(r_powers, 2)
            * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
            * (
                -1
                + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2)
            )
            * width
            * power(x_powers, 4)
            - 32
            * torch.pow(-1.0 + power(halfZ_powers, 2) * power(r_powers, 2), 2)
            * (
                -1
                + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2)
            )
            * power(x_powers, 5)
        )
        * 100000000.0
    )

    b = (
        (-1 + (halfX_powers[1] + halfZ_powers[1]) * r_powers[1])
        * (
            -(1 + halfZ_powers[0] * r_powers[0]) * width_powers[1]
            + 4 * halfX_powers[0] * r_powers[0] * width_powers[0] * x_powers[0]
            + 4 * (-1 + halfZ_powers[0] * r_powers[0]) * x_powers[1]
        )
        * (
            (1 - halfZ_powers[0] * r_powers[0]) * width_powers[1]
            + 4 * halfX_powers[0] * r_powers[0] * width_powers[0] * x_powers[0]
            + 4 * (1 + halfZ_powers[0] * r_powers[0]) * x_powers[1]
        )
    ) * 100000000.0

    return a / b


def lineShade(lower, upper, alpha, halfX, halfZ, width):
    r = torch.sqrt(1 - alpha * alpha)

    width_powers = calc_power_series(width)
    halfX_powers = calc_power_series(halfX)
    halfZ_powers = calc_power_series(halfZ)
    r_powers = calc_power_series(r)

    temp = torch.sqrt(
        -power(width_powers, 2)
        + power(halfX_powers, 2) * power(r_powers, 2) * power(width_powers, 2)
        + power(halfZ_powers, 2) * power(r_powers, 2) * power(width_powers, 2)
    )

    c = [
        (-(halfX * power(r_powers, 1) * width) - temp)
        / (2.0 * (-1 + halfZ * power(r_powers, 1))),
        (-(halfX * power(r_powers, 1) * width) - temp)
        / (2.0 * (1 + halfZ * power(r_powers, 1))),
        (-(halfX * power(r_powers, 1) * width) + temp)
        / (2.0 * (-1 + halfZ * power(r_powers, 1))),
        (-(halfX * power(r_powers, 1) * width) + temp)
        / (2.0 * (1 + halfZ * power(r_powers, 1))),
    ]

    ret = torch.zeros_like(lower, dtype=torch.complex64)

    for i in range(4):
        part = sumpart(
            lower, upper, c[i], width_powers, halfX_powers, halfZ_powers, r_powers
        )
        ret += part
    ret *= (
        power(r_powers, 2) * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) * width
    ) / ((-1 + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2)))

    ret += calc_res(
        upper, width_powers, halfX_powers, halfZ_powers, r_powers
    ) - calc_res(lower, width_powers, halfX_powers, halfZ_powers, r_powers)

    coeff = (
        -alpha
        * alpha
        / (
            (
                8.0
                * torch.pi
                * torch.pow(-1.0 + power(halfZ_powers, 2) * power(r_powers, 2), 3)
            )
        )
    )

    ret *= coeff

    return ret.real


def FrSchlick(R0, cosTheta):
    return R0 + (1.0 - R0) * torch.pow(1.0 - cosTheta, 5)


def DisneyFresnel(R0, metallic, eta, cosI):
    return FrSchlick(R0, cosI)


import glints.microfacet as microfacet


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

    line_width = width.item()

    temp = (
        lineShade(
            torch.max(minimum, torch.tensor(-line_width, device=minimum.device)),
            torch.min(maximum, torch.tensor(line_width, device=maximum.device)),
            torch.sqrt(
                torch.complex(glints_roughness, torch.zeros_like(glints_roughness))
            ),
            half_vec[:, 0],
            half_vec[:, 2],
            line_width,
        )
        / torch.norm(light_pos_uv - p, dim=1)
        / torch.norm(light_pos_uv - p, dim=1)
    )

    torch.set_printoptions(precision=10)
    temp *= microfacet.bsdf_f_line(camera_dir, light_dir, glints_roughness)

    # Assuming bsdf_f_line is a function defined elsewhere

    area = intersect_area(lines, patches, 2.0 * width)

    # print(area)
    patch_area = torch.abs(cross_2d(p1 - p0, p2 - p0) / 2.0) + torch.abs(
        cross_2d(p2 - p0, p3 - p0) / 2.0
    )

    mask = (
        (minimum * maximum > 0)
        & (torch.abs(minimum) > line_width)
        & (torch.abs(maximum) > line_width)
    )

    result = torch.where(
        mask,
        torch.tensor(0.0, device=temp.device),
        temp
        * area
        / patch_area
        / torch.abs(
            torch.max(minimum, torch.tensor(-line_width, device=minimum.device))
            - torch.min(maximum, torch.tensor(line_width, device=maximum.device))
        ),
    )
    

    return torch.stack((result, area), dim=1)


import glints.bspline as bspline


# line ctr_points: [n, 3, 2]
# patch shape: [n, 4, 2]
# cam_positions shape: [n, 3]
# light_positions shape: [n, 3]
# glints_roughness shape: [1]
# width shape: [1]
def ShadeBSplineElements(
    ctr_points, patches, cam_positions, light_positions, glints_roughness, width
):
    assert ctr_points.shape[0] == patches.shape[0]
    patch_center = (
        patches[:, 0, :] + patches[:, 1, :] + patches[:, 2, :] + patches[:, 3, :]
    ) / 4.0

    t_closest = bspline.calc_closest(patch_center, ctr_points)
    p = bspline.eval_quadratic_bspline_point(ctr_points, t_closest)
    tangent = bspline.eval_quadratic_bspline_tangent(ctr_points, t_closest)
    assert not torch.isnan(p).any(), "Point contains NaN values"
    assert not torch.isnan(tangent).any(), "Tangent contains NaN values"



    end1 = p - tangent * 0.02
    end2 = p + tangent * 0.02

    lines = torch.stack((end1, end2), dim=1)

    ret =  ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )
    # nan_mask = torch.isnan(ret)
    # print ("nan pairs count: ", torch.sum(nan_mask))

    ret = torch.nan_to_num(ret, nan=0.0)
    
    return ret
