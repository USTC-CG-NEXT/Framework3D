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