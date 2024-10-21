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


def exec_node(x, y, z):

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

    # plydata = PlyData.read(ply_path)

    cache["xyz"] = np.array([[x, y, z]], dtype=np.float32)
    cache["opacity"] = 1 * np.ones((cache["xyz"].shape[0], 1), dtype=np.float32)
    cache["trbf_center"] = np.zeros((cache["xyz"].shape[0], 1), dtype=np.float32)
    cache["trbf_scale"] = np.ones((cache["xyz"].shape[0], 1), dtype=np.float32)
    cache["motion"] = np.ones((cache["xyz"].shape[0], 9), dtype=np.float32)
    cache["features_dc"] = 1.0 * np.ones((cache["xyz"].shape[0], 6), dtype=np.float32)
    cache["features_dc"][:, 0] = 0.0
    cache["scales"] = 0 * np.ones((cache["xyz"].shape[0], 3), dtype=np.float32)
    cache["rots"] = np.ones((cache["xyz"].shape[0], 4), dtype=np.float32)
    cache["rots"][0][0] = 1.0
    cache["rots"][0][1] = 1.0

    cache["omegas"] = np.zeros((cache["xyz"].shape[0], 1), dtype=np.float32)
    cache["fts"] = np.ones((cache["xyz"].shape[0], 1), dtype=np.float32)

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
        {"x": "Float", "y": "Float", "z": "Float"},
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


def wrap_exec(list):
    return exec_node(*list)
