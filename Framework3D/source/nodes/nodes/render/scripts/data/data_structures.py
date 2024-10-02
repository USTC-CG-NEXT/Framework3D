import numpy as np
import torch
torch.set_default_device('cuda')
from typing import NamedTuple

class CameraInfo(NamedTuple):
    idx: int
    R: np.array
    T: np.array
    fovy: np.array
    fovx: np.array
    image: torch.tensor
    world_view_transform: torch.tensor
    projection_matrix: torch.tensor
    full_proj_transform: torch.tensor
    camera_center: torch.tensor
    time : float
    rays: torch.tensor
    event_image: torch.tensor
    future_cams: list

class SimpleCameraInfo(NamedTuple):
    R: np.array
    T: np.array
    fovy: np.array
    fovx: np.array
    world_view_transform: torch.tensor
    projection_matrix: torch.tensor
    full_proj_transform: torch.tensor
    camera_center: torch.tensor
    time: float

class PointCloud(NamedTuple):
    points: np.array
    colors: np.array
    normals: np.array
   
class SceneInfo(NamedTuple):
    point_cloud: PointCloud
    train_cameras: list
    test_cameras: list
    video_cameras: list
    radius: float
    max_time: float
    times: np.array