# script.py
import torch
import math

from data.data_structures import CameraInfo
import torch.nn.functional as F

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
            "h": "Int",
            "w": "Int",
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


from diff_gaussian_rasterization_stg import (
    GaussianRasterizationSettings as GaussianRasterizationSettingsSTG,
    GaussianRasterizer as GaussianRasterizerSTG,
)


def exec_node(
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

    raster_settings = GaussianRasterizationSettingsSTG(
        image_height=h,
        image_width=w,
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

    expanded_motion = (
        motion[:, :3] * rel_time
        + motion[:, 3:6] * rel_time**2
        + motion[:, 6:9] * rel_time**3
    )
    xyz = xyz + expanded_motion

    v = motion[:, :3] + 2 * motion[:, 3:6] * rel_time + 3 * motion[:, 6:9] * rel_time**2


    print(type(cam.world_view_transform))
    print((cam.world_view_transform.shape))
    world_view_transform_reshaped = cam.world_view_transform.reshape(4, 4)
    # compute 2d velocity
    r1, r2, r3 = torch.chunk(world_view_transform_reshaped.T[:3, :3], 3, dim=0)
    t1, t2, t3 = (
        world_view_transform_reshaped.T[0, 3],
        world_view_transform_reshaped.T[1, 3],
        world_view_transform_reshaped.T[2, 3],
    )
    projection_matrix_reshaped = cam.projection_matrix.reshape(4, 4)
    mx, my = projection_matrix_reshaped[0, 0], projection_matrix_reshaped[1, 1]
    vx = (r1 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r1 @ xyz.T + t1.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2
    vy = (r2 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r2 @ xyz.T + t2.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2

    newly_added = 0

    dmotion = torch.cat(
        (
            vx * mx * w * 0.5 * 0.25,
            vy * my * h * 0.5 * 0.25            
        ),
        dim=0,
    ).permute(1, 0)

    colors_precomp = torch.cat((features_dc, rel_time * features_t, dmotion), dim=1)

    print("xyz", xyz.shape)
    print("screenspace_points", screenspace_points.shape)
    print("colors_precomp", colors_precomp.shape)
    print("opacity", opacity.shape)
    print("scale", scale.shape)
    print("rotation", rotation.shape)

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


def wrap_exec(list):
    return exec_node(*list)
