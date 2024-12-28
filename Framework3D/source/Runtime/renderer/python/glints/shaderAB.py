import  torch


# line shape: [n, 2, 2]
# patch shape: [n, 4, 2]
# cam_positions shape: [n, 3]
# light_positions shape: [n, 3]
# glints_roughness shape: [1]
# width shape: [1]
def ShadeLineElementAB(
    lines, patches, cam_positions, light_positions, glints_roughness, width
):
    camera_pos_uv = cam_positions
    light_pos_uv = light_positions

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

    points = torch.stack(
        [
            signed_area(lines, p0),
            signed_area(lines, p1),
            signed_area(lines, p2),
            signed_area(lines, p3),
        ],
        dim=1,
    )

    minimum = torch.min(points, dim=1).values
    maximum = torch.max(points, dim=1).values

    cut = 0.4
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
            points[:, 1],
            points[:, 2],
            points[:, 3],
            points[:, 0],
        ],
        dim=1,
    )

    lower = torch.stack(
        [
            points[:, 0],
            points[:, 1],
            points[:, 2],
            points[:, 3],
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
            torch.sqrt(glints_roughness),
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

    return torch.stack((result, glints_area), dim=1)