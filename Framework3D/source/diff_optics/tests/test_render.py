import pytest

from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock


def test_shader_compile(shader_path):
    import sys

    sys.stderr.write(f"Current file path: {__file__}\n")

    import slangtorch
    import os

    os.environ["TORCH_CUDA_ARCH_LIST"] = "8.6"

    lens_system = LensSystem()
    lens_system.set_default()
    compiler = LensSystemCompiler()
    compiled, block = compiler.compile(lens_system, False)
    with open("lens_shader.slang", "w") as file:
        file.write(compiled)
    m = slangtorch.loadModule(
        shader_path + "physical_lens_raygen_torch.slang",
        includePaths=[shader_path, "."],
    )
    assert m is not None
