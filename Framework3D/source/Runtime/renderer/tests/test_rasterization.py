import glints.rasterization as rast
import torch
import numpy as np
import nvdiffrast.torch as dr


def test_projection_rasterization():
    import glints.rasterization as rast

    v, i = rast.get_triangles()  # v.shape: [1,1,n,4], i.shape: [m,3]
    glctx = dr.RasterizeCudaContext()

    # projection matrix, 4x4
    projection_matrix = rast.perspective_projection(0.1, 10.0, 120.0)

    print("projection_matrix", projection_matrix)

    # 4x4 matrix
    view_matrix = rast.look_at(
        np.array([0.5, 0.5, 80.0]),
        np.array([0.5, 0.5, 0.0]),
        np.array([1.0, 1.0, 0.0]),
    )

    print("view_matrix", view_matrix)

    v_transformed = torch.matmul(
        torch.matmul(
            projection_matrix,
            view_matrix,
        ),
        v,
    )

    print(v_transformed.shape)  # torch.Size([1, 4, 4])
    print(v_transformed)

    rasterized, _ = dr.rasterize(glctx, v_transformed, i, resolution=[512, 512])
    print(rasterized.shape)  # torch.Size([1, 512, 512, 4])
    import imageio

    imageio.imwrite(
        "rasterized_projection.png",
        (rasterized[0, ..., :3].cpu().numpy() * 255).astype(np.uint8),
    )
