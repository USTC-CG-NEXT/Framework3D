import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), f"../python")))


def test_solve_cubic():
    import glints.bspline
    import torch

    a = torch.tensor([1.0]).cuda()
    b = torch.tensor([3.0]).cuda()
    c = torch.tensor([2.0]).cuda()
    d = torch.tensor([1.0]).cuda()
    t_roots_real, t_roots_imag = glints.bspline.solve_cubic_eqn(a, b, c, d)
    print(t_roots_real)
    print(t_roots_imag)


def test_closest_point():
    import glints.bspline
    import torch

    p = torch.tensor([[-1.5, -3.0]]).cuda()
    p = torch.tensor(
        [
            [-1.5, -3.0],
            [0.0, 1.0],
            [2.0, 3.0],
            [4.0, 1.0],
            [5.0, -2.0],
            [2.0, -4.0],
            [1.0, 2.0],
            [3.0, -1.0],
            [4.5, 8.5],
            [6.0, -3.0],
        ]
    ).cuda()
    ctr_points = torch.tensor([[[0.0, 0.0], [2.0, 2.0], [4.0, 0.0]]]).cuda()

    ctr_points = ctr_points.repeat(p.shape[0], 1, 1)

    t = glints.bspline.calc_closest(p, ctr_points).cuda()
    print(t)

    position = glints.bspline.eval_quadratic_bspline_point(ctr_points, t)
    tangent = glints.bspline.eval_quadratic_bspline_tangent(ctr_points, t)

    print(position)
    print("tangent", tangent)


import torch


def test_plot_b_spline():
    import glints.bspline
    import matplotlib.pyplot as plt

    t = torch.linspace(-1, 4, steps=100).cuda()
    y = glints.bspline.quadratic_piecewise_bspline(t).cpu().numpy()

    plt.plot(t.cpu().numpy(), y)
    plt.xlabel("t")
    plt.ylabel("B-Spline Value")
    plt.title("Quadratic Piecewise B-Spline")
    plt.grid(True)
    # plt.show()
