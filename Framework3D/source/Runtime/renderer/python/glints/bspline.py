import torch


def solve_cubic_eqn(a, b, c, d):
    """
    Solves a cubic equation of the form ax^3 + bx^2 + cx + d = 0 for real roots.
    Parameters:
    a (torch.Tensor): Coefficient tensor of shape [n] for the x^3 term.
    b (torch.Tensor): Coefficient tensor of shape [n] for the x^2 term.
    c (torch.Tensor): Coefficient tensor of shape [n] for the x term.
    d (torch.Tensor): Coefficient tensor of shape [n] for the constant term.
    Returns:
    torch.Tensor: A tensor of shape [n, 3] containing the real parts of the 3 roots for each equation.
    """

    # Coefficients a, b, c, d are all tensors of shape [n]
    # The result will be a tensor of shape [n, 3] for the 3 roots
    delta0 = b**2 - 3 * a * c
    delta1 = 2 * b**3 - 9 * a * b * c + 27 * a**2 * d
    discriminant = delta1**2 - 4 * delta0**3
    discriminant = discriminant.to(torch.complex64)

    C = ((delta1 + torch.sqrt(discriminant)) / 2) ** (1 / 3)
    C[discriminant.real < 0] = ((delta1 - torch.sqrt(discriminant)) / 2) ** (1 / 3)

    xi = torch.tensor(
        [
            -0.5 + 0.5j * torch.sqrt(torch.tensor(3.0)),
            -0.5 - 0.5j * torch.sqrt(torch.tensor(3.0)),
            1.0,
        ],
        dtype=torch.complex64,
    )

    roots = torch.zeros((a.shape[0], 3), dtype=torch.complex64)
    for i in range(3):
        roots[:, i] = -1 / (3 * a) * (b + xi[i] * C + delta0 / (xi[i] * C))

    return roots.real, roots.imag


def calc_closest(p, ctr_points):
    """
    Calculate the closest points on B-spline curves to a given set of points.

    Args:
        p (torch.Tensor): A tensor of shape [n, 2] representing the set of points.
        ctr_points (torch.Tensor): A tensor of shape [n, 3, 2] representing the control points of the B-spline curves.

    Returns:
        torch.Tensor: A tensor of the closest points on the B-spline curves to the given set of points, represented with the curve parameter t.
    """
    # Extract control points
    x0, y0 = ctr_points[:, 0, 0], ctr_points[:, 0, 1]
    x1, y1 = ctr_points[:, 1, 0], ctr_points[:, 1, 1]
    x2, y2 = ctr_points[:, 2, 0], ctr_points[:, 2, 1]
    px, py = p[:, 0], p[:, 1]

    # Coefficients for the cubic equation in t
    a = (
        x0**2
        - 4 * x0 * x1
        + 4 * x1**2
        + 2 * x0 * x2
        - 4 * x1 * x2
        + x2**2
        + y0**2
        - 4 * y0 * y1
        + 4 * y1**2
        + 2 * y0 * y2
        - 4 * y1 * y2
        + y2**2
    )
    b = -3 * (
        2 * x0**2
        - 7 * x0 * x1
        + 6 * x1**2
        + 3 * x0 * x2
        - 5 * x1 * x2
        + x2**2
        + 2 * y0**2
        - 7 * y0 * y1
        + 6 * y1**2
        + 3 * y0 * y2
        - 5 * y1 * y2
        + y2**2
    )

    c = (
        12 * x0**2
        - 35 * x0 * x1
        + 24 * x1**2
        + 13 * x0 * x2
        - 17 * x1 * x2
        + 3 * x2**2
        - 2 * px * (x0 - 2 * x1 + x2)
        - 2 * py * y0
        + 12 * y0**2
        + 4 * py * y1
        - 35 * y0 * y1
        + 24 * y1**2
        - 2 * py * y2
        + 13 * y0 * y2
        - 17 * y1 * y2
        + 3 * y2**2
    )
    d = (
        4 * px * x0
        - 8 * x0**2
        - 6 * px * x1
        + 18 * x0 * x1
        - 9 * x1**2
        + 2 * px * x2
        - 6 * x0 * x2
        + 6 * x1 * x2
        - x2**2
        + 4 * py * y0
        - 8 * y0**2
        - 6 * py * y1
        + 18 * y0 * y1
        - 9 * y1**2
        + 2 * py * y2
        - 6 * y0 * y2
        + 6 * y1 * y2
        - y2**2
    )

    # Solve the cubic equation for t
    t_roots_real, t_roots_imag = solve_cubic_eqn(a, b, c, d)

    t_clamped = torch.clamp(t_roots_real, torch.tensor(1.0), torch.tensor(2.0))
    # Mask out the roots where the imaginary part is not zero
    valid_mask = torch.isclose(t_roots_imag, torch.zeros_like(t_roots_imag))
    t_roots_real = t_roots_real * valid_mask + torch.logical_not(valid_mask) * 1e10

    # Select the closest point on the B-spline curve
    closest_t = torch.min(t_roots_real, dim=1).values

    return closest_t
