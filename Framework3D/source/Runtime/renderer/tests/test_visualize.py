import os
import ast
import hd_USTC_CG_py
import numpy as np
import torch
import glints.shaderAB as shader
import glints.renderer as renderer
import glints.test_utils as test_utils
import glints.rasterization as rasterization
import imageio


def load_lines():
    # read a list of list from a txt
    with open("stroke.txt", "r") as f:
        lines_file = ast.literal_eval(f.read())
        end_points = []
        for line in lines_file:
            for i in range(len(line) - 1):
                end_points.append([line[i], line[i + 1]])

        lines = torch.tensor(end_points, device="cuda")

        lines = torch.cat(
            (lines, torch.zeros((lines.shape[0], 2, 1), device="cuda")), dim=2
        )

    return lines


import torch


def test_load_lines():

    print(load_lines().shape)


def test_render_directed_scratches():
    r = renderer.Renderer()
    r.scratch_context.set_max_pair_buffer_ratio(15.0)
    lines = load_lines()
    r.set_width(torch.tensor([0.001], device="cuda"))
    r.set_glints_roughness(torch.tensor([0.002], device="cuda"))

    vertices, indices = renderer.plane_board_scene_vertices_and_indices()
    vertex_buffer_stride = 5 * 4
    resolution = [1536 * 2, 1024 * 2]
    camera_position_np = np.array([2, 2, 3], dtype=np.float32)
    light_position_np = np.array([2, -2, 3], dtype=np.float32)

    r.set_camera_position(camera_position_np)
    r.set_light_position(light_position_np)
    r.set_perspective(np.pi / 3, resolution[0] / resolution[1], 0.1, 1000.0)
    r.set_mesh(vertices, indices, vertex_buffer_stride)


    for i in range(30):
        angle = i * (2 * np.pi / 2)
        rotation_matrix = np.array(
            [
                [np.cos(angle), -np.sin(angle), 0],
                [np.sin(angle), np.cos(angle), 0],
                [0, 0, 1],
            ],
            dtype=np.float32,
        )
        rotated_camera_position = np.dot(rotation_matrix, camera_position_np)
        r.set_camera_position(rotated_camera_position)
        image, _ = r.render(resolution, lines)
        image *= 10
        save_image(image, resolution, f"raster_test/directed_intersection_{i}.png")
