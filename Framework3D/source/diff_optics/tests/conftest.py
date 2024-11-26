import sys
import os

def pytest_addoption(parser):
    parser.addoption(
        "--target", action="store", default="default_target", help="target directory"
    )


def pytest_configure(config):
    target = config.getoption("target")
    sys.path.append(
        os.path.abspath(
            os.path.join(os.path.dirname(__file__), f"../../../Binaries/{target}")
        )
    )
    
    print(f"Added {os.path.abspath(os.path.join(os.path.dirname(__file__), f'../../../Binaries/{target}'))} to sys.path")