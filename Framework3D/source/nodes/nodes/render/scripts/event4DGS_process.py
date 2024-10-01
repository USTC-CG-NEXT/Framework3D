# script.py
import torch
import math

from data.data_structures import CameraInfo


def render_stg(
    cam,
    xyz,
    trbf_center,
    trbf_scale,
    active_sh_degree,
    opacity,
    scale,
    rotation,
    omega,
    features_dc,
    features_t,
    motion,
    h=None,
    w=None,
):
    screenspace_points = (
        torch.zeros_like(
            xyz,
            dtype=xyz.dtype,
            requires_grad=True,
            device=xyz.device,
        )
        + 0
    )
    try:
        screenspace_points.retain_grad()
    except:
        pass

    time = torch.tensor(cam.time).to(xyz.device).repeat(xyz.shape[0], 1)
    tanfovx = math.tan(cam.fovx * 0.5)
    tanfovy = math.tan(cam.fovy * 0.5)

    if h is None:
        img_h = int(cam.image.size(1))
    else:
        img_h = h
    if w is None:
        img_w = int(cam.image.size(2))
    else:
        img_w = w

    raster_settings = GaussianRasterizationSettingsSTG(
        image_height=img_h,
        image_width=img_w,
        tanfovx=tanfovx,
        tanfovy=tanfovy,
        bg=torch.tensor(
            [0.0, 0.0, 0.0], device=xyz.device
        ),  # torch.tensor([1., 1., 1.], device=xyz.device),
        scale_modifier=1.0,
        viewmatrix=cam.world_view_transform.to(xyz.device),
        projmatrix=cam.full_proj_transform.to(xyz.device),
        sh_degree=active_sh_degree,
        campos=cam.camera_center.to(xyz.device),
        prefiltered=False,
    )
    rasterizer = GaussianRasterizerSTG(raster_settings=raster_settings)

    rel_time = (time - trbf_center).detach()

    opacity = torch.sigmoid(opacity)
    trbf_dist = (time - trbf_center) / torch.exp(trbf_scale)
    trbf = torch.exp(-1 * trbf_dist.pow(2))
    opacity = opacity * trbf

    scale = torch.exp(scale)
    rotation = F.normalize(rotation + rel_time * omega)

    motion = (
        motion[:, :3] * rel_time
        + motion[:, 3:6] * rel_time**2
        + motion[:, 6:9] * rel_time**3
    )
    xyz = xyz + motion

    v = motion[:, :3] + 2 * motion[:, 3:6] * rel_time + 3 * motion[:, 6:9] * rel_time**2

    # compute 2d velocity
    r1, r2, r3 = torch.chunk(cam.world_view_transform.T[:3, :3], 3, dim=0)
    t1, t2, t3 = (
        cam.world_view_transform.T[0, 3],
        cam.world_view_transform.T[1, 3],
        cam.world_view_transform.T[2, 3],
    )
    mx, my = cam.projection_matrix[0, 0], cam.projection_matrix[1, 1]
    vx = (r1 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r1 @ xyz.T + t1.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2
    vy = (r2 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r2 @ xyz.T + t2.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2
    dmotion = torch.cat(
        (
            vx * mx * img_w * 0.5 * 0.25,
            vy * my * img_h * 0.5 * 0.25,
            newly_added.view(1, -1),
        ),
        dim=0,
    ).permute(1, 0)

    colors_precomp = torch.cat((features_dc, rel_time * features_t, dmotion), dim=1)

    # rasterize visible Gaussians to image, obtain their radii (on screen).
    rendered_results, radii, depth = rasterizer(
        means3D=xyz,
        means2D=screenspace_points,
        shs=None,
        colors_precomp=colors_precomp,
        opacities=opacity,
        scales=scale,
        rotations=rotation,
        cov3D_precomp=None,
    )

    rendered_feature, rendered_motion, newly_added = (
        rendered_results[:-3, :, :],
        rendered_results[-3:-1, :, :],
        rendered_results[-1, :, :],
    )

    rendered_image = rendered_feature.unsqueeze(0)[3, :]
    # rendered_image = decoder(
    #     rendered_feature.unsqueeze(0), cam.rays.to(xyz.device)
    # )

    if event_decoder is None:
        events = None
    else:
        events = rendered_motion.unsqueeze(0)

    return {
        "render": rendered_image,
        "viewspace_points": screenspace_points,
        "visibility_filter": radii > 0,
        "radii": radii,
        "depth": depth,
        "opacity": opacity,
        "events": events,
        "newly_added": newly_added,
    }


def declare_node():
    return [
        {
            "cam": "PyObj",
            "xyz": "TorchTensor",
            "trbf_center": "TorchTensor",
            "trbf_scale": "TorchTensor",
            "active_sh_degree": "Int",
            "opacity": "TorchTensor",
            "scale": "TorchTensor",
            "rotation": "TorchTensor",
            "omega": "TorchTensor",
            "features_dc": "TorchTensor",
            "features_t": "TorchTensor",
            "motion": "TorchTensor",
        },
        {
            "render": "TorchTensor",
            "viewspace_points": "TorchTensor",
            "visibility_filter": "TorchTensor",
            "radii": "TorchTensor",
            "depth": "TorchTensor",
            "opacity": "TorchTensor",
            "events": "TorchTensor",
            "newly_added": "TorchTensor",
        },
    ]


def exec_node(
    xyz,
    trbf_center,
    trbf_scale,
    active_sh_degree,
    opacity,
    scale,
    rotation,
    omega,
    features_dc,
    features_t,
    motion,
):
    # print("a ", type(a))
    # print("b ", type(b))

    # c =torch.tensor(a,device='cuda')
    # d = c+b

    cam = CameraInfo()

    (
        render,
        viewspace_points,
        visibility_filter,
        radii,
        depth,
        opacity,
        events,
        newly_added,
    ) = render_stg(
        cam,
        xyz,
        trbf_center,
        trbf_scale,
        active_sh_degree,
        opacity,
        scale,
        rotation,
        omega,
        features_dc,
        features_t,
        motion,
    )

    return (
        render,
        viewspace_points,
        visibility_filter,
        radii,
        depth,
        opacity,
        events,
        newly_added,
    )


def wrap_exec(list):
    return exec_node(*list)
