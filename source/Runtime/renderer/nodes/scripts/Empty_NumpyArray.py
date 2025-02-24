def declare_node():
    return [{}, {"array": "NumpyArray"}]


def wrap_exec(list):
    return exec_node(*list)


import numpy as np


def exec_node():
    return np.empty(1)
