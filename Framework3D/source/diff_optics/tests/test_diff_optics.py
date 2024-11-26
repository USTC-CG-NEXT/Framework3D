import pytest


from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock


def test_lens_system_initialization():
    lens_system = LensSystem()
    assert lens_system.lens_count() == 0


# def test_lens_system_add_lens():
#     lens_system = LensSystem()
#     lens_system.add_lens("lens1")
#     assert lens_system.lens_count() == 1


# def test_lens_system_deserialize_string():
#     lens_system = LensSystem()
#     lens_system.deserialize("lens_data_string")
#     assert lens_system.lens_count() > 0


# def test_lens_system_deserialize_path():
#     lens_system = LensSystem()
#     lens_system.deserialize("/path/to/lens_data")
#     assert lens_system.lens_count() > 0


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
    with open("py_lens_shader.slang", "w") as f:
        f.write(compiled)


# def test_lens_system_compiler_emit_line():
#     compiler = LensSystemCompiler()
#     compiler.emit_line("line_data")
#     # Assuming emit_line has some effect we can check


# def test_lens_system_compiler_compile():
#     compiler = LensSystemCompiler()
#     compiler.compile()
#     # Assuming compile has some effect we can check


# def test_lens_system_compiler_fill_block_data():
#     block_data = LensSystemCompiler.fill_block_data()
#     assert block_data is not None


# def test_compiled_data_block_initialization():
#     data_block = CompiledDataBlock()
#     assert data_block.parameters == []
#     assert data_block.parameter_offsets == []
#     assert data_block.cb_size == 0


# def test_compiled_data_block_rw():
#     data_block = CompiledDataBlock()
#     data_block.parameters = [1, 2, 3]
#     data_block.parameter_offsets = [0, 1, 2]
#     data_block.cb_size = 3
#     assert data_block.parameters == [1, 2, 3]
#     assert data_block.parameter_offsets == [0, 1, 2]
#     assert data_block.cb_size == 3
