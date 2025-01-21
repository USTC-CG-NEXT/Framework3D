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
    r.scratch_context.set_max_pair_buffer_ratio(30.0)
    lines = load_lines()
    r.set_width(torch.tensor([0.0005], device="cuda"))
    r.set_glints_roughness(torch.tensor([0.0016/4], device="cuda"))

    vertices, indices = renderer.plane_board_scene_vertices_and_indices()
    vertex_buffer_stride = 5 * 4
    resolution = [1000, 800]
    camera_position_np = np.array([0, 1.0, 6], dtype=np.float32)
    light_position_np = np.array([0, 6, 4], dtype=np.float32)

    r.set_look_at(camera_position_np, [0, 0, 0], [0, 1, 0])
    r.set_light_position(light_position_np)
    r.set_perspective(np.pi / 6, resolution[0] / resolution[1], 0.1, 1000.0)
    r.set_mesh(vertices, indices, vertex_buffer_stride)

    translate_range = [-3, 3]

    test_view_count = 30
    translation_step = (translate_range[1] - translate_range[0]) / test_view_count

    rotation_range = [-np.pi * 0.1475837423, np.pi * 0.1475837423]
    rotation_step = (rotation_range[1] - rotation_range[0]) / test_view_count
    for i in range(test_view_count):

        translation = np.array(
            [translate_range[0] + i * translation_step, 0, 0], dtype=np.float32
        )
        translated_camera_position = test_utils.translate_position(
            camera_position_np, translation
        )

        angle = rotation_range[0] + i * rotation_step

        rotated_camera_position = test_utils.rotate_postion(
            camera_position_np, angle, axis=np.array([0, 1, 0], dtype=np.float32)
        )
        r.set_look_at(rotated_camera_position, [0, 0, 0], [0, 1, 0])
        image, _ = r.render(resolution, lines)
        image *= 10
        test_utils.save_image(
            image, resolution, f"raster_test/directed_intersection_{i}.exr"
        )
        torch.cuda.empty_cache()
