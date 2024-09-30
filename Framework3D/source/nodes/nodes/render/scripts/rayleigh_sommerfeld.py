import numpy as np
import drjit as dr

def compute(arr: np.ndarray):
    arr = arr * 1.0
    arr[:, :, 3] = 1
    return arr
