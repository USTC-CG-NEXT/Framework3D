import pytest


from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock


def test_lens_system_initialization():
    lens_system = LensSystem()
    assert lens_system.lens_count() == 0


def test_lens_system_set_default():
    lens_system = LensSystem()
    lens_system.set_default()
    assert lens_system.lens_count() == 12


def test_lens_system_compiler_initialization():
    lens_system = LensSystem()
    lens_system.set_default()
    compiler = LensSystemCompiler()
    assert compiler is not None
    compiled, block = compiler.compile(lens_system, True)
    assert compiled is not None
    assert block is not None
    with open("lens_shader.slang", "w") as file:
        file.write(compiled)


def test_shader_compile():
    import slangtorch
    import os
    os.environ['TORCH_CUDA_ARCH_LIST'] = '8.6'

    lens_system = LensSystem()
    lens_system.set_default()
    compiler = LensSystemCompiler()
    compiled, block = compiler.compile(lens_system, True)
    with open("lens_shader.slang", "w") as file:
        file.write(compiled)
    m = slangtorch.loadModule("lens_shader.slang", includePaths=["./usd/hd_USTC_CG/resources/shaders"])
    assert m is not None
    