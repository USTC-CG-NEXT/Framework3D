import pytest

#  from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock


# def test_shader_compile(shader_path):
#     import sys

#     sys.stderr.write(f"Current file path: {__file__}\n")

#     import slangtorch
#     import os

#     os.environ["TORCH_CUDA_ARCH_LIST"] = "8.6"

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


# def test_mitsuba_render():
#     import mitsuba as mi

#     mi.set_variant("cuda_ad_rgb")
#     img = mi.render(mi.load_dict(mi.cornell_box()), spp=10240)
#     mi.Bitmap(img).write("cbox.exr")


def test_custom_raygen():
    import mitsuba as mi

    mi.set_variant("cuda_ad_rgb")

    import LensCamera
    import importlib

    importlib.reload(LensCamera)

    import os

    file_directory = os.path.dirname(os.path.abspath(__file__))
    file_path = file_directory + "/"
    scene = mi.load_file(file_path + "./teapot.xml")

    mi.render(scene)
    original_image = mi.render(scene, spp=128)

    mi.Bitmap(original_image).write("test.exr")


if __name__ == "__main__":
    test_custom_raygen()
