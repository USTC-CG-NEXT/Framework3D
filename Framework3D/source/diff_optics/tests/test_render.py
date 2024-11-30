import pytest
import LensCamera
import mitsuba as mi
import os

file_directory = os.path.dirname(os.path.abspath(__file__))
file_path = file_directory + "/"

from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock
import slangtorch

from struct import pack, unpack

import torch
import sys


# def test_shader_compile(shader_path):
#     LensCamera.set_shader_path(shader_path)

#     lens_system = LensSystem()
#     lens_system.set_default()
#     compiler = LensSystemCompiler()
#     compiled, block = compiler.compile(lens_system, False)
#     with open("lens_shader.slang", "w") as file:
#         file.write(compiled)
#     m = slangtorch.loadModule(
#         shader_path + "physical_lens_raygen_torch.slang",
#         includePaths=[shader_path, "."],
#     )

#     assert m is not None

def test_shader_run(shader_path):
    LensCamera.set_shader_path(shader_path)

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

    random_seeds = torch.randint(0, 1, (10, 1), device="cuda", dtype=torch.int32)
    data_tensor_size = block.cb_size
    data_tensor = torch.zeros(data_tensor_size, device="cuda", dtype=torch.float32)

    for i, p in enumerate(block.parameters):
        data_tensor[i] = p

    data_tensor[0] = 36
    data_tensor[1] = 1
    data_tensor[2] = unpack("f", pack("i", 10))[0]
    data_tensor[3] = unpack("f", pack("i", 10))[0]
    data_tensor[4] = 10

    print(data_tensor, file=sys.stderr)
    print(data_tensor.shape)

    rays = torch.zeros(10, 1, 11, device="cuda", dtype=torch.float32)
    pixel_targets = torch.zeros(10, 1, 2, device="cuda", dtype=torch.int32)

    m.computeMain(
        random_seeds=random_seeds,
        lens_system_data_tensor=data_tensor,
        rays=rays,
        pixel_targets=pixel_targets,
    ).launchRaw(blockSize=(32, 1, 1), gridSize=(64, 1, 1))
    print(rays, file=sys.stderr)
    print(pixel_targets, file=sys.stderr)

    assert m is not None


# def test_shader_compile_shortcut(shader_path):
#     m = LensCamera.shader_compile(shader_path)
#     LensCamera.set_shader_path(shader_path)

#     assert m is not None


# def test_custom_raygen(shader_path):
#     LensCamera.set_shader_path(shader_path=shader_path)
#     scene = mi.load_file(file_path + "./teapot.xml")
#     mi.render(scene)
#     original_image = mi.render(scene)
#     mi.Bitmap(original_image).write("test.exr")
