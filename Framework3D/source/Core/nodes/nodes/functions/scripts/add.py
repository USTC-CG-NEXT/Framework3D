def declare_node():
    return [{"a": "Float", "b": "Float"}, {"c": "Float"}]


def wrap_exec(list):
    return exec_node(*list)


def exec_node(a, b):
    return a + b
