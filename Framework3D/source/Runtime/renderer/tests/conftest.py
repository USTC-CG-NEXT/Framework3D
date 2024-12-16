import sys
import os
import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--target", action="store", default="default_target", help="target directory"
    )
    parser.addoption(
        "--shipping", action="store_true", help="set shader path for shipping"
    )


def pytest_configure(config):
    target = config.getoption("target")
    shipping = config.getoption("shipping")

    target_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), f"../../../../Binaries/{target}")
    )
    sys.path.append(target_path)
    print(f"Added {target_path} to sys.path")

    package_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), f"../python")
    )
    sys.path.append(package_path)

    print(f"Added {package_path} to sys.path")

    os.chdir(target_path)

    if shipping:
        shader_path = "./usd/hd_USTC_CG/resources/shaders/"
    else:
        shader_path = "../../source/renderer/nodes/shaders/shaders/"

    config._shader_path = shader_path  # Store shader_path in config object

    print(f"Added {target_path} to sys.path")
    print(f"Shader path set to {shader_path}")


@pytest.fixture
def shader_path(request):
    return request.config._shader_path
