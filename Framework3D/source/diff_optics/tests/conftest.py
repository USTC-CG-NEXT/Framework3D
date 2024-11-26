import sys
import os

def pytest_addoption(parser):
    parser.addoption(
        "--target", action="store", default="default_target", help="target directory"
    )


def pytest_configure(config):
    target = config.getoption("target")
    target_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), f"../../../Binaries/{target}")
    )
    sys.path.append(target_path)
    os.chdir(target_path)
    
    print(f"Added {os.path.abspath(os.path.join(os.path.dirname(__file__), f'../../../Binaries/{target}'))} to sys.path")