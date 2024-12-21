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

    p = torch.tensor([[2.0, 3.0]]).cuda()
    ctr_points = torch.tensor([[[0.0, 0.0], [2.0, 2.0], [4.0, 0.0]]]).cuda()
    t = glints.bspline.calc_closest(p, ctr_points).cuda()
    print(t)

    print(glints.bspline.quadratic_piecewise_bspine(torch.tensor([0.5])))

    print(glints.bspline.eval_quadratic_bspline_point(ctr_points, t))
