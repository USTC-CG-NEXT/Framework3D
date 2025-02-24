# script.py
import torch

torch.set_default_device("cuda")
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
    from diff_gaussian_rasterization_stg import (
        GaussianRasterizationSettings as GaussianRasterizationSettingsSTG,
        GaussianRasterizer as GaussianRasterizerSTG,
    )

    assert xyz.is_cuda, "fuck, xyz is not on CUDA device"
    screenspace_points = (
        torch.zeros_like(
            xyz,
            dtype=xyz.dtype,
            device=xyz.device,
        )
        + 0
    )

    time = torch.tensor(cam.time).to(xyz.device).repeat(xyz.shape[0], 1)

    tanfovx = math.tan(cam.fovx * 0.5)
    tanfovy = math.tan(cam.fovy * 0.5)

    world_view_transform_reshaped = cam.world_view_transform.reshape(4, 4)
    projection_matrix_reshaped = cam.full_proj_transform.reshape(4, 4)
    
    #print(world_view_transform_reshaped)
    point = torch.tensor([0, 0, 0, 1], dtype=torch.float32, device="cuda")
    # print(world_view_transform_reshaped.inverse())
    # print((torch.mv(world_view_transform_reshaped, point)))
    #print(world_view_transform_reshaped)
    #print((torch.mv(projection_matrix_reshaped, point)))

    raster_settings = GaussianRasterizationSettingsSTG(
        image_height=h,
        image_width=w,
        tanfovx=tanfovx,
        tanfovy=tanfovy,
        bg=torch.tensor(
            [0.0, 0.0, 0.0], device=xyz.device, dtype=torch.float32
        ).contiguous(),  # torch.tensor([1., 1., 1.], device=xyz.device),
        scale_modifier=1,
        viewmatrix=world_view_transform_reshaped.contiguous(),
        projmatrix=projection_matrix_reshaped.contiguous(),
        sh_degree=active_sh_degree,
        campos=cam.camera_center.to(xyz.device).contiguous(),
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

    # compute 2d velocity
    r1, r2, r3 = torch.chunk(world_view_transform_reshaped.T[:3, :3], 3, dim=0)
    t1, t2, t3 = (
        world_view_transform_reshaped.T[0, 3],
        world_view_transform_reshaped.T[1, 3],
        world_view_transform_reshaped.T[2, 3],
    )

    # Ensure all tensors have dtype float
    assert xyz.is_cuda, "xyz is not of dtype float"
    assert trbf_center.is_cuda, "trbf_center is not of dtype float"
    assert trbf_scale.is_cuda, "trbf_scale is not of dtype float"
    assert opacity.is_cuda, "opacity is not of dtype float"
    assert scale.is_cuda, "scale is not of dtype float"
    assert rotation.is_cuda, "rotation is not of dtype float"
    assert omega.is_cuda, "omega is not of dtype float"
    assert features_dc.is_cuda, "features_dc is not of dtype float"
    assert features_t.is_cuda, "features_t is not of dtype float"
    assert motion.is_cuda, "motion is not of dtype float"

    mx, my = projection_matrix_reshaped[0, 0], projection_matrix_reshaped[1, 1]
    vx = (r1 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r1 @ xyz.T + t1.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2
    vy = (r2 @ v.T) / (r3 @ xyz.T + t3.view(1, 1)) - (r3 @ v.T) * (
        r2 @ xyz.T + t2.view(1, 1)
    ) / (r3 @ xyz.T + t3.view(1, 1)) ** 2

    newly_added = 0

    dmotion = torch.cat(
        (vx * mx * w * 0.5 * 0.25, vy * my * h * 0.5 * 0.25),
        dim=0,
    ).permute(1, 0)

    colors_precomp = torch.cat((features_dc, rel_time * features_t, dmotion), dim=1)

    assert (
        xyz.is_cuda and xyz.dtype == torch.float32
    ), "xyz is not on CUDA device or not of dtype float32"
    assert (
        screenspace_points.is_cuda and screenspace_points.dtype == torch.float32
    ), "screenspace_points is not on CUDA device or not of dtype float32"
    assert (
        colors_precomp.is_cuda and colors_precomp.dtype == torch.float32
    ), "colors_precomp is not on CUDA device or not of dtype float32"
    assert (
        opacity.is_cuda and opacity.dtype == torch.float32
    ), "opacity is not on CUDA device or not of dtype float32"
    assert (
        scale.is_cuda and scale.dtype == torch.float32
    ), "scale is not on CUDA device or not of dtype float32"
    assert (
        rotation.is_cuda and rotation.dtype == torch.float32
    ), "rotation is not on CUDA device or not of dtype float32"

    rendered_results, radii, depth = rasterizer(
        means3D=xyz.contiguous(),
        means2D=screenspace_points.contiguous(),
        shs=None,
        colors_precomp=colors_precomp.contiguous(),
        opacities=opacity.contiguous(),
        scales=scale.contiguous(),
        rotations=rotation.contiguous(),
        cov3D_precomp=None,
    )

    rendered_feature, rendered_motion, newly_added = (
        rendered_results[:-3, :, :],
        rendered_results[-3:-1, :, :],
        rendered_results[-1, :, :],
    )

    rgb = rendered_feature[:3, :, :].permute(1, 2, 0).contiguous()
    # add one channel of 1 to the rendered image
    rendered_image = torch.cat(
        ((rgb), torch.ones_like(rgb[:, :, :1])), dim=2
    )

    rendered_image = rendered_feature.unsqueeze(0)

    events = torch.zeros(1, device=xyz.device)

    torch.cuda.empty_cache()
    return (
        rendered_image,
        screenspace_points,
        radii > 0,
        radii,
        depth,
        opacity,
        events,
        newly_added,
    )


def wrap_exec(list):
    return exec_node(*list)
