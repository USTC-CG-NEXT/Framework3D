import torch
import glints.microfacet as microfacet

work_for_div = 100000000


def cross_2d(a, b):
    return a[:, 0] * b[:, 1] - a[:, 1] * b[:, 0]


def signed_area(lines, points):
    line_pos = (lines[:, 0, :] + lines[:, 1, :]) / 2.0
    line_direction = torch.nn.functional.normalize(lines[:, 1, :] - lines[:, 0, :])
    distance = points - line_pos

    x = cross_2d(distance, line_direction)
    y = torch.sum(distance * line_direction, dim=1)

    return torch.stack((x, y), dim=1)


def slope(p1, p2):
    return (p1[:, 1] - p2[:, 1]) / (p1[:, 0] - p2[:, 0] + 1e-9)


def intercept(p1, p2):
    return (p1[:, 0] * p2[:, 1] - p2[:, 0] * p1[:, 1]) / (p1[:, 0] - p2[:, 0] + 1e-9)


def calc_power_series(tensor):
    powers = [tensor]
    for i in range(1, 8):
        powers.append(powers[-1] * tensor)
    return powers


def power(tensor_powers, n):
    return tensor_powers[n - 1]


def calc_res_a(x, a, b, width_powers, halfX_powers, halfZ_powers, r_powers):
    x_powers = calc_power_series(x)
    halfX = power(halfX_powers, 1)
    halfZ = power(halfZ_powers, 1)
    r = power(r_powers, 1)
    width = power(width_powers, 1)

    rest = 16 * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) * (
        b * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
        - 4 * a * halfX * halfZ * power(r_powers, 2) * width
    ) * x + 8 * a * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 2 * power(
        x_powers, 2
    )

    m = (
        4
        * power(r_powers, 2)
        * power(width_powers, 2)
        * (
            -2
            * b
            * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
            * (
                halfX
                * halfZ
                * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
                * (
                    -2
                    + (3 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                    * power(r_powers, 2)
                    + power(halfZ_powers, 2)
                    * (power(halfX_powers, 2) + power(halfZ_powers, 2))
                    * power(r_powers, 4)
                )
                * power(width_powers, 3)
                - 2
                * (
                    power(halfZ_powers, 2)
                    * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 3
                    + 4
                    * power(halfX_powers, 4)
                    * power(halfZ_powers, 2)
                    * power(r_powers, 4)
                    * (3 + power(halfZ_powers, 2) * power(r_powers, 2))
                    + power(halfX_powers, 2)
                    * (
                        1
                        - 9 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 3 * power(halfZ_powers, 4) * power(r_powers, 4)
                        + 5 * power(halfZ_powers, 6) * power(r_powers, 6)
                    )
                )
                * power(width_powers, 2)
                * x
                + 4
                * halfX
                * halfZ
                * (
                    2
                    + 3
                    * (-5 * power(halfX_powers, 2) + 3 * power(halfZ_powers, 2))
                    * power(r_powers, 2)
                    + 6
                    * (
                        2 * power(halfX_powers, 4)
                        - power(halfX_powers, 2) * power(halfZ_powers, 2)
                        - 2 * power(halfZ_powers, 4)
                    )
                    * power(r_powers, 4)
                    + power(halfZ_powers, 2)
                    * (power(halfX_powers, 2) + power(halfZ_powers, 2))
                    * (4 * power(halfX_powers, 2) + power(halfZ_powers, 2))
                    * power(r_powers, 6)
                )
                * width
                * power(x_powers, 2)
                + 8
                * (
                    (1 + power(halfZ_powers, 2) * power(r_powers, 2))
                    * (halfZ - power(halfZ_powers, 3) * power(r_powers, 2)) ** 2
                    + 2
                    * power(halfX_powers, 4)
                    * (
                        power(r_powers, 2)
                        + 6 * power(halfZ_powers, 2) * power(r_powers, 4)
                        + power(halfZ_powers, 4) * power(r_powers, 6)
                    )
                    + power(halfX_powers, 2)
                    * (
                        -1
                        - 11 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 9 * power(halfZ_powers, 4) * power(r_powers, 4)
                        + 3 * power(halfZ_powers, 6) * power(r_powers, 6)
                    )
                )
                * power(x_powers, 3)
            )
            + a
            * width
            * (
                32
                * power(halfX_powers, 6)
                * power(r_powers, 4)
                * (
                    1
                    + 6 * power(halfZ_powers, 2) * power(r_powers, 2)
                    + power(halfZ_powers, 4) * power(r_powers, 4)
                )
                * width
                * power(x_powers, 2)
                + power(halfZ_powers, 2)
                * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 2
                * width
                * (
                    (-1 + power(halfZ_powers, 4) * power(r_powers, 4))
                    * power(width_powers, 2)
                    - 4
                    * (
                        1
                        + 6 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(x_powers, 2)
                )
                + 16
                * power(halfX_powers, 5)
                * halfZ
                * power(r_powers, 4)
                * x
                * (
                    -(
                        1
                        + 6 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(width_powers, 2)
                    + 2
                    * (
                        5
                        + 10 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(x_powers, 2)
                )
                + 2
                * halfX
                * halfZ
                * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 2
                * x
                * (
                    (
                        2
                        - 5 * power(halfZ_powers, 2) * power(r_powers, 2)
                        - 5 * power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(width_powers, 2)
                    + 4
                    * (
                        2
                        + 15 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 3 * power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(x_powers, 2)
                )
                - 2
                * power(halfX_powers, 3)
                * halfZ
                * power(r_powers, 2)
                * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
                * x
                * (
                    (
                        1
                        + 50 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 13 * power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(width_powers, 2)
                    - 4
                    * (
                        19
                        + 54 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 7 * power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(x_powers, 2)
                )
                + 2
                * power(halfX_powers, 4)
                * power(r_powers, 2)
                * width
                * (
                    (
                        -1
                        - 5 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 5 * power(halfZ_powers, 4) * power(r_powers, 4)
                        + power(halfZ_powers, 6) * power(r_powers, 6)
                    )
                    * power(width_powers, 2)
                    + 8
                    * (
                        -2
                        - 21 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 4 * power(halfZ_powers, 4) * power(r_powers, 4)
                        + 3 * power(halfZ_powers, 6) * power(r_powers, 6)
                    )
                    * power(x_powers, 2)
                )
                + power(halfX_powers, 2)
                * width
                * (
                    (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 2
                    * (
                        1
                        + 12 * power(halfZ_powers, 2) * power(r_powers, 2)
                        + 3 * power(halfZ_powers, 4) * power(r_powers, 4)
                    )
                    * power(width_powers, 2)
                    + 4
                    * (
                        1
                        + 38 * power(halfZ_powers, 2) * power(r_powers, 2)
                        - 12 * power(halfZ_powers, 4) * power(r_powers, 4)
                        - 30 * power(halfZ_powers, 6) * power(r_powers, 6)
                        + 3 * power(halfZ_powers, 8) * power(r_powers, 8)
                    )
                    * power(x_powers, 2)
                )
            )
        )
    ) * work_for_div

    n = (
        (-1 + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2))
        * (
            (1 + halfZ * r) * power(width_powers, 2)
            - 4 * halfX * r * width * x
            - 4 * (-1 + halfZ * r) * power(x_powers, 2)
        )
        * (
            (-1 + halfZ * r) * power(width_powers, 2)
            - 4 * halfX * r * width * x
            - 4 * (1 + halfZ * r) * power(x_powers, 2)
        )
    ) * work_for_div

    return rest + m / n


def sumpart_coeff_b(y, width_powers, halfX_powers, halfZ_powers, r_powers):
    y_powers = calc_power_series(y)

    m = (
        (
            (power(halfZ_powers, 1) - power(halfZ_powers, 3) * power(r_powers, 2)) ** 2
            + power(halfX_powers, 2)
            * (-1 + power(halfZ_powers, 4) * power(r_powers, 4))
        )
        * power(width_powers, 3)
        - 4
        * power(halfX_powers, 1)
        * power(halfZ_powers, 1)
        * (
            -2
            + (3 * power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2)
            + power(halfZ_powers, 2)
            * (power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 4)
        )
        * power(width_powers, 2)
        * power(y_powers, 1)
        + 4
        * (
            -7 * power(halfX_powers, 2)
            + 7 * power(halfZ_powers, 2)
            + 2
            * (
                3 * power(halfX_powers, 4)
                - 2 * power(halfX_powers, 2) * power(halfZ_powers, 2)
                - 4 * power(halfZ_powers, 4)
            )
            * power(r_powers, 2)
            + power(halfZ_powers, 2)
            * (power(halfX_powers, 2) + power(halfZ_powers, 2))
            * (2 * power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 4)
        )
        * power(width_powers, 1)
        * power(y_powers, 2)
        + 64
        * power(halfX_powers, 1)
        * power(halfZ_powers, 1)
        * (-1 + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2))
        * power(y_powers, 3)
    )

    n = (
        power(halfX_powers, 1)
        * power(halfZ_powers, 1)
        * power(r_powers, 2)
        * power(width_powers, 3)
        + 2
        * (
            1
            + (-2 * power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 2)
        )
        * power(width_powers, 2)
        * power(y_powers, 1)
        - 12
        * power(halfX_powers, 1)
        * power(halfZ_powers, 1)
        * power(r_powers, 2)
        * power(width_powers, 1)
        * power(y_powers, 2)
        - 8 * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) * power(y_powers, 3)
    )

    assert torch.isnan(m).sum() == 0
    assert torch.isnan(n).sum() == 0

    return -m / n


def sumpart_coeff_a(y, width_powers, halfX_powers, halfZ_powers, r_powers):
    y_powers = calc_power_series(y)

    m = (
        halfX_powers[0]
        * halfZ_powers[0]
        * (-1 + power(halfZ_powers, 2) * power(r_powers, 2))
        * (
            -10
            + (11 * power(halfX_powers, 2) + 9 * power(halfZ_powers, 2))
            * power(r_powers, 2)
            + power(halfZ_powers, 2)
            * (power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 4)
        )
        * power(width_powers, 3)
        - 4
        * (
            power(halfZ_powers, 2)
            * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) ** 3
            + 2
            * power(halfX_powers, 4)
            * power(halfZ_powers, 2)
            * power(r_powers, 4)
            * (11 + power(halfZ_powers, 2) * power(r_powers, 2))
            + power(halfX_powers, 2)
            * (
                1
                - 21 * power(halfZ_powers, 2) * power(r_powers, 2)
                + 17 * power(halfZ_powers, 4) * power(r_powers, 4)
                + 3 * power(halfZ_powers, 6) * power(r_powers, 6)
            )
        )
        * power(width_powers, 2)
        * y_powers[0]
        + 4
        * halfX_powers[0]
        * halfZ_powers[0]
        * (
            22
            + 3
            * (-23 * power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 2)
            + 2
            * (
                22 * power(halfX_powers, 4)
                + 7 * power(halfX_powers, 2) * power(halfZ_powers, 2)
                - 14 * power(halfZ_powers, 4)
            )
            * power(r_powers, 4)
            + power(halfZ_powers, 2)
            * (power(halfX_powers, 2) + power(halfZ_powers, 2))
            * (4 * power(halfX_powers, 2) + 3 * power(halfZ_powers, 2))
            * power(r_powers, 6)
        )
        * power(width_powers, 1)
        * power(y_powers, 2)
        + 64
        * (-1 + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2))
        * (
            power(halfX_powers, 2)
            - power(halfZ_powers, 2)
            + power(halfZ_powers, 2)
            * (5 * power(halfX_powers, 2) + power(halfZ_powers, 2))
            * power(r_powers, 2)
        )
        * power(y_powers, 3)
    ) * work_for_div

    assert torch.isnan(m).sum() == 0

    n = (
        4
        * power(halfX_powers, 2)
        * power(r_powers, 2)
        * power(width_powers, 2)
        * y_powers[0]
        + 8 * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) * power(y_powers, 3)
        - 2
        * power(width_powers, 2)
        * (y_powers[0] + power(halfZ_powers, 2) * power(r_powers, 2) * y_powers[0])
        - halfX_powers[0]
        * halfZ_powers[0]
        * power(r_powers, 2)
        * power(width_powers, 1)
        * (power(width_powers, 2) - 12 * power(y_powers, 2))
    ) * work_for_div

    assert torch.isnan(n).sum() == 0

    return m / n


def lineShade(lower, upper, a, b, alpha, halfX, halfZ, width):

    assert torch.isnan(a).sum() == 0
    r = torch.sqrt(1 - alpha * alpha)

    width_powers = calc_power_series(width)
    halfX_powers = calc_power_series(halfX)
    halfZ_powers = calc_power_series(halfZ)
    halfX_powers_expanded = calc_power_series(halfX.unsqueeze(1))
    halfZ_powers_expanded = calc_power_series(halfZ.unsqueeze(1))
    r_powers = calc_power_series(r)

    temp = torch.sqrt(
        -power(width_powers, 2)
        + power(halfX_powers, 2) * power(r_powers, 2) * power(width_powers, 2)
        + power(halfZ_powers, 2) * power(r_powers, 2) * power(width_powers, 2)
    )

    assert torch.isnan(temp).sum() == 0

    c = torch.stack(
        [
            (-(halfX * r * width) - temp) / (2 * (-1 + halfZ * r)),
            (-(halfX * r * width) - temp) / (2 * (1 + halfZ * r)),
            (-(halfX * r * width) + temp) / (2 * (-1 + halfZ * r)),
            (-(halfX * r * width) + temp) / (2 * (1 + halfZ * r)),
        ],
        dim=0,
    )

    assert torch.isnan(c).sum() == 0

    # Vectorized operations
    coeff_b = sumpart_coeff_b(c, width_powers, halfX_powers, halfZ_powers, r_powers)

    coeff_a = sumpart_coeff_a(c, width_powers, halfX_powers, halfZ_powers, r_powers)

    # Reshape upper and lower: [n,4] -> [n,4,4] by repeating for each c
    upper_expanded = upper.unsqueeze(1).repeat(1, 4, 1)  # [n,4,4]
    lower_expanded = lower.unsqueeze(1).repeat(1, 4, 1)  # [n,4,4]

    # Reshape c: [4,n] -> [n,4,4] by repeating and transposing
    c_expanded = c.transpose(0, 1).unsqueeze(2).repeat(1, 1, 4)  # [n,4,4]

    log_val_u = torch.log(upper_expanded - c_expanded)
    log_val_l = torch.log(lower_expanded - c_expanded)

    assert torch.isnan(log_val_u).sum() == 0
    assert torch.isnan(log_val_l).sum() == 0
    assert torch.isnan(a).sum() == 0
    assert torch.isnan(coeff_a).sum() == 0

    # Reshape coeff_a/b to match log_val dimensions [n,4,4]
    coeff_a = coeff_a.transpose(0, 1).unsqueeze(2).repeat(1, 1, 4)
    coeff_b = coeff_b.transpose(0, 1).unsqueeze(2).repeat(1, 1, 4)

    # First multiply coefficients with log differences
    part_a = (log_val_u - log_val_l) * coeff_a  # [n,4,4]
    part_b = (log_val_u - log_val_l) * coeff_b  # [n,4,4]

    # Then multiply by a/b and sum across both dimensions
    part_a = part_a * a.unsqueeze(1)  # [n,4,4]
    part_b = part_b * b.unsqueeze(1)  # [n,4,4]

    ret_a = torch.sum(part_a, dim=(1, 2))  # [n]
    ret_b = torch.sum(part_b, dim=(1, 2))  # [n]

    assert torch.isnan(ret_a).sum() == 0
    assert torch.isnan(ret_b).sum() == 0

    temp_1 = (
        power(r_powers, 2) * (-1 + power(halfZ_powers, 2) * power(r_powers, 2)) * width
    ) / (-1 + (power(halfX_powers, 2) + power(halfZ_powers, 2)) * power(r_powers, 2))

    ret_b *= temp_1
    ret_a *= temp_1 * width

    # Vectorize res calculation
    res = calc_res_a(
        upper,
        a,
        b,
        width_powers,
        halfX_powers_expanded,
        halfZ_powers_expanded,
        r_powers,
    ) - calc_res_a(
        lower,
        a,
        b,
        width_powers,
        halfX_powers_expanded,
        halfZ_powers_expanded,
        r_powers,
    )

    res = torch.sum(res, dim=(1))

    ret_a += res

    coeff_b = (
        -alpha
        * alpha
        / (
            8
            * torch.pi
            * torch.pow(-1 + power(halfZ_powers, 2) * power(r_powers, 2), 3)
        )
    )
    coeff_a = torch.pow(alpha, 2) / (
        16 * torch.pi * torch.pow(-1 + power(halfZ_powers, 2) * power(r_powers, 2), 4)
    )
    ret_a *= coeff_a
    ret_b *= coeff_b

    return ret_b.real + ret_a.real


def areaIntegrate(x, a, b):
    return 0.5 * a * x * x + b * x


def areaCalc(lower, upper, a, b):
    upper_area = areaIntegrate(upper, a, b)
    lower_area = areaIntegrate(lower, a, b)
    ret = torch.sum(upper_area - lower_area, dim=1)
    return torch.abs(ret)


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
    torch.set_printoptions(precision=10)

    camera_pos_uv = cam_positions.cuda()
    light_pos_uv = light_positions.cuda()

    p0 = patches[:, 0, :]
    p1 = patches[:, 1, :]
    p2 = patches[:, 2, :]
    p3 = patches[:, 3, :]

    center = (p0 + p1 + p2 + p3) / 4.0

    p = torch.stack(
        (
            center[:, 0],
            center[:, 1],
            torch.zeros(center.shape[0], device=center.device),
        ),
        dim=1,
    )

    camera_dir = torch.nn.functional.normalize(camera_pos_uv - p, dim=1)
    light_dir = torch.nn.functional.normalize(light_pos_uv - p, dim=1)

    cam_dir_2D = camera_dir[:, :2]
    light_dir_2D = light_dir[:, :2]

    line_direction = torch.nn.functional.normalize(lines[:, 1, :] - lines[:, 0, :])

    local_cam_dir = torch.nn.functional.normalize(
        torch.stack(
            (
                cross_2d(cam_dir_2D, line_direction),
                torch.sum(cam_dir_2D * line_direction, dim=1),
                camera_dir[:, 2],
            ),
            dim=1,
        ),
        dim=1,
    )

    local_light_dir = torch.nn.functional.normalize(
        torch.stack(
            (
                cross_2d(light_dir_2D, line_direction),
                torch.sum(light_dir_2D * line_direction, dim=1),
                light_dir[:, 2],
            ),
            dim=1,
        ),
        dim=1,
    )

    half_vec = torch.nn.functional.normalize(local_cam_dir + local_light_dir, dim=1)

    points = torch.stack(
        [
            signed_area(lines, p0),
            signed_area(lines, p1),
            signed_area(lines, p2),
            signed_area(lines, p3),
        ],
        dim=1,
    )

    minimum = torch.min(points[:, :, 0], dim=1).values
    maximum = torch.max(points[:, :, 0], dim=1).values

    cut = 0.5
    left_cut = -cut * width
    right_cut = cut * width

    a = torch.stack(
        [
            slope(points[:, 0], points[:, 1]),
            slope(points[:, 1], points[:, 2]),
            slope(points[:, 2], points[:, 3]),
            slope(points[:, 3], points[:, 0]),
        ],
        dim=1,
    )

    assert torch.isnan(a).sum() == 0

    b = torch.stack(
        [
            intercept(points[:, 0], points[:, 1]),
            intercept(points[:, 1], points[:, 2]),
            intercept(points[:, 2], points[:, 3]),
            intercept(points[:, 3], points[:, 0]),
        ],
        dim=1,
    )

    upper = torch.stack(
        [
            points[:, 1, 0],
            points[:, 2, 0],
            points[:, 3, 0],
            points[:, 0, 0],
        ],
        dim=1,
    )

    lower = torch.stack(
        [
            points[:, 0, 0],
            points[:, 1, 0],
            points[:, 2, 0],
            points[:, 3, 0],
        ],
        dim=1,
    )

    upper = torch.where(upper >= right_cut, right_cut, upper)
    upper = torch.where(upper <= left_cut, left_cut, upper)

    lower = torch.where(lower >= right_cut, right_cut, lower)
    lower = torch.where(lower <= left_cut, left_cut, lower)

    temp = (
        lineShade(
            lower,
            upper,
            a,
            b,
            torch.sqrt(
                torch.complex(glints_roughness, torch.zeros_like(glints_roughness))
            ),
            half_vec[:, 0],
            half_vec[:, 2],
            width,
        )
        / torch.norm(light_pos_uv - p, dim=1)
        / torch.norm(light_pos_uv - p, dim=1)
        * microfacet.bsdf_f_line(camera_dir, light_dir, glints_roughness)
    )

    patch_area = torch.abs(cross_2d(p1 - p0, p2 - p0) / 2.0) + torch.abs(
        cross_2d(p2 - p0, p3 - p0) / 2.0
    )

    mask = (
        (minimum * maximum > 0)
        & (torch.abs(minimum) > cut * width)
        & (torch.abs(maximum) > cut * width)
    )

    result = torch.where(
        mask,
        torch.tensor(0.0, device=temp.device),
        torch.abs(temp) / patch_area,
    )

    glints_area = areaCalc(lower, upper, a, b)

    assert torch.isnan(result).sum() == 0

    return torch.stack((result, glints_area), dim=1)


import glints.bspline as bspline


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

    ret = ShadeLineElement(
        lines, patches, cam_positions, light_positions, glints_roughness, width
    )

    assert not torch.isnan(ret).any()
    # nan_mask = torch.isnan(ret)
    # print ("nan pairs count: ", torch.sum(nan_mask))

    return ret
