import os

file_path = os.path.abspath(".")
execution_path = os.path.abspath(f"../../../../Binaries/Debug")

os.chdir(file_path)
import sys

target_path = os.path.abspath(f"../../../../Binaries/Debug")
sys.path.append(target_path)
print(f"Added {target_path} to sys.path")
package_path = os.path.abspath(f"../python")
sys.path.append(package_path)
print(f"Added {package_path} to sys.path")


import glints.scratch_grid
import glints.renderer
import glints.test_utils as test_utils
import torch
import numpy as np
import pytest
import argparse


def draw_master_direction_field(cam_pos, light_pos):
    return


def draw_master_direction_fields(cam_poses, light_poses):
    return


if __name__ == "__main__":
    r = glints.renderer.Renderer()

    def parse_args():
        parser = argparse.ArgumentParser(description="Master Direction Visualization")
        parser.add_argument(
            "--cam_pos",
            type=float,
            nargs=3,
            required=True,
            help="Camera position as [x, y, z]",
        )
        parser.add_argument(
            "--light_pos",
            type=float,
            nargs=3,
            required=True,
            help="Light position as [x, y, z]",
        )
        return parser.parse_args()

    args = parse_args()
    cam_pos = args.cam_pos
    light_pos = args.light_pos

    draw_master_direction_field(cam_pos, light_pos)
