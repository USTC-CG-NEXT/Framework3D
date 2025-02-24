
import numpy as np

cache = {
    "xyz": None,
    "opacity": None,
    "trbf_center": None,
    "trbf_scale": None,
    "motion": None,
    "features_dc": None,
    "scales": None,
    "rots": None,
    "omegas": None,
    "fts": None,
}


def read_model(ply_path):
    from plyfile import PlyData, PlyElement
    if all(value is not None for value in cache.values()):
        return (
            cache["xyz"],
            cache["opacity"],
            cache["trbf_center"],
            cache["trbf_scale"],
            cache["motion"],
            cache["features_dc"],
            cache["scales"],
            cache["rots"],
            cache["omegas"],
            cache["fts"],
        )

    plydata = PlyData.read(ply_path)
    cache["xyz"] = np.stack(
        (
            np.asarray(plydata.elements[0]["x"]),
            np.asarray(plydata.elements[0]["y"]),
            np.asarray(plydata.elements[0]["z"]),
        ),
        axis=1,
    ).astype(np.float32)
    cache["opacity"] = np.asarray(plydata.elements[0]["opacity"])[
        ..., np.newaxis
    ].astype(np.float32)
    cache["trbf_center"] = np.asarray(plydata.elements[0]["trbf_center"])[
        ..., np.newaxis
    ].astype(np.float32)
    cache["trbf_scale"] = np.asarray(plydata.elements[0]["trbf_scale"])[
        ..., np.newaxis
    ].astype(np.float32)

    cache["motion"] = np.zeros((cache["xyz"].shape[0], 9), dtype=np.float32)
    for i in range(9):
        cache["motion"][:, i] = np.asarray(
            plydata.elements[0]["motion_{}".format(i)]
        ).astype(np.float32)

    cache["features_dc"] = np.zeros((cache["xyz"].shape[0], 6), dtype=np.float32)
    for i in range(6):
        cache["features_dc"][:, i] = np.asarray(
            plydata.elements[0]["f_dc_{}".format(i)]
        ).astype(np.float32)

    scale_names = [
        p.name for p in plydata.elements[0].properties if p.name.startswith("scale_")
    ]
    scale_names = sorted(scale_names, key=lambda x: int(x.split("_")[-1]))
    cache["scales"] = np.zeros(
        (cache["xyz"].shape[0], len(scale_names)), dtype=np.float32
    )
    for idx, attr_name in enumerate(scale_names):
        cache["scales"][:, idx] = np.asarray(plydata.elements[0][attr_name]).astype(
            np.float32
        )

    rot_names = [
        p.name for p in plydata.elements[0].properties if p.name.startswith("rot")
    ]
    rot_names = sorted(rot_names, key=lambda x: int(x.split("_")[-1]))
    cache["rots"] = np.zeros((cache["xyz"].shape[0], len(rot_names)), dtype=np.float32)
    for idx, attr_name in enumerate(rot_names):
        cache["rots"][:, idx] = np.asarray(plydata.elements[0][attr_name]).astype(
            np.float32
        )

    omega_names = [
        p.name for p in plydata.elements[0].properties if p.name.startswith("omega")
    ]
    cache["omegas"] = np.zeros(
        (cache["xyz"].shape[0], len(omega_names)), dtype=np.float32
    )
    for idx, attr_name in enumerate(omega_names):
        cache["omegas"][:, idx] = np.asarray(plydata.elements[0][attr_name]).astype(
            np.float32
        )

    ft_names = [
        p.name for p in plydata.elements[0].properties if p.name.startswith("f_t")
    ]
    cache["fts"] = np.zeros((cache["xyz"].shape[0], len(ft_names)), dtype=np.float32)
    for idx, attr_name in enumerate(ft_names):
        cache["fts"][:, idx] = np.asarray(plydata.elements[0][attr_name]).astype(
            np.float32
        )
    all_names = [
        p.name for p in plydata.elements[0].properties
    ]
    return (
        cache["xyz"],
        cache["opacity"],
        cache["trbf_center"],
        cache["trbf_scale"],
        cache["motion"],
        cache["features_dc"],
        cache["scales"],
        cache["rots"],
        cache["omegas"],
        cache["fts"],
    )


def declare_node():
    return [
        {
            "File": "String",
        },
        {
            "xyz": "NumpyArray",
            "opacity": "NumpyArray",
            "trbf_center": "NumpyArray",
            "trbf_scale": "NumpyArray",
            "motion": "NumpyArray",
            "features_dc": "NumpyArray",
            "scales": "NumpyArray",
            "rots": "NumpyArray",
            "omegas": "NumpyArray",
            "fts": "NumpyArray",
        },
    ]


def exec_node(File):
    return read_model(File)


def wrap_exec(list):
    return exec_node(*list)
