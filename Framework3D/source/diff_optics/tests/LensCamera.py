from diff_optics_py import LensSystem, LensSystemCompiler, CompiledDataBlock

global _shader_path
import os

os.environ["TORCH_CUDA_ARCH_LIST"] = "8.6"
import slangtorch
import torch

from struct import pack, unpack


def set_shader_path(shader_path):
    global _shader_path
    _shader_path = shader_path


def get_shader_path():
    global _shader_path
    return _shader_path


def shader_compile(shader_path):

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
    return m, block


import mitsuba as mi

mi.set_variant("cuda_ad_rgb")

import drjit as dr


class LensCamera(mi.ProjectiveCamera):
    def __init__(self, props):
        mi.Sensor.__init__(self, props)

        global _shader_path
        self.m, self.block_cb = shader_compile(shader_path=_shader_path)
        assert self.m is not None

        size = self.film().size()
        self.x_fov = mi.parse_fov(props, size.x / size.y)
        self.to_world = mi.Transform4d(props.get("to_world"))
        if self.to_world.has_scale():
            raise Exception(
                "Scale factors in the camera-to-world transformation are not allowed!"
            )

        self.principal_point_offset = mi.Point2f(
            props.get("principal_point_offset_x", 0.0),
            props.get("principal_point_offset_y", 0.0),
        )

        self.resolution = self.film().crop_size()

        # Distance to the near clipping plane
        self.near_clip = props.get("near_clip", 1e-2)
        # Distance to the far clipping plane
        self.far_clip = props.get("far_clip", 1e4)
        # Distance to the focal plane
        self.focus_distance = props.get("focus_distance", self.far_clip)

        if self.near_clip <= 0.0:
            raise Exception("The 'near_clip' parameter must be greater than zero!")
        if self.near_clip >= self.far_clip:
            raise Exception(
                "The 'near_clip' parameter must be smaller than 'far_clip'."
            )

        self.update_camera_transforms()

    def traverse(self, callback):
        super().traverse(callback)
        callback.put_parameter(
            "x_fov",
            self.x_fov,
            mi.ParamFlags.Differentiable | mi.ParamFlags.Discontinuous,
        )
        callback.put_parameter(
            "principal_point_offset_x",
            self.principal_point_offset.x,
            mi.ParamFlags.Differentiable | mi.ParamFlags.Discontinuous,
        )
        callback.put_parameter(
            "principal_point_offset_y",
            self.principal_point_offset.y,
            mi.ParamFlags.Differentiable | mi.ParamFlags.Discontinuous,
        )
        callback.put_parameter(
            "to_world",
            self.to_world,
            mi.ParamFlags.Differentiable | mi.ParamFlags.Discontinuous,
        )

    def parameters_changed(self, keys):
        super().parameters_changed(keys)
        if not keys or "to_world" in keys:
            if self.to_world.has_scale():
                raise Exception(
                    "Scale factors in the camera-to-world transformation are not allowed!"
                )

        self.update_camera_transforms()

    def update_camera_transforms(self):

        self.camera_to_sample = mi.perspective_projection(
            self.film().size(),
            self.film().crop_size(),
            self.film().crop_offset(),
            self.x_fov,
            self.near_clip,
            self.far_clip,
        )

        self.sample_to_camera = self.camera_to_sample.inverse()
        print(self.sample_to_camera)

        self.dx = self.sample_to_camera @ mi.Point3f(
            1.0 / self.resolution.x, 0.0, 0.0
        ) - self.sample_to_camera @ mi.Point3f(0.0)
        self.dy = self.sample_to_camera @ mi.Point3f(
            0.0, 1.0 / self.resolution.y, 0.0
        ) - self.sample_to_camera @ mi.Point3f(0.0)

        pmin = self.sample_to_camera @ mi.Point3f(0.0, 0.0, 0.0)
        pmax = self.sample_to_camera @ mi.Point3f(1.0, 1.0, 0.0)

        self.image_rect = mi.BoundingBox2f()
        self.image_rect.expand(mi.Point2f(pmin.x, pmin.y) / pmin.z)
        self.image_rect.expand(mi.Point2f(pmax.x, pmax.y) / pmax.z)
        self.normalization = 1.0 / self.image_rect.volume()
        self.needs_sample_3 = False

        dr.make_opaque(
            self.camera_to_sample,
            self.sample_to_camera,
            self.dx,
            self.dy,
            self.x_fov,
            self.image_rect,
            self.normalization,
            self.principal_point_offset,
        )

    def run_shader(self, random, data_tensor, rays, pixel_targets):
        shape = rays.shape
        self.m.computeMain(
            random_seeds=random,
            lens_system_data_tensor=data_tensor,
            rays=rays,
            pixel_targets=pixel_targets,
        ).launchRaw(
            blockSize=(32, 32, 1), gridSize=(shape[0] // 32 + 1, shape[1] // 32 + 1, 1)
        )
        return rays

    def sample_ray(
        self, time, wavelength_sample, position_sample, aperture_sample, active=True
    ):
        wavelengths, wav_weight = self.sample_wavelengths(
            dr.zeros(mi.SurfaceInteraction3f), wavelength_sample, active
        )
        ray = mi.Ray3f()
        ray.time = time
        ray.wavelengths = wavelengths

        scaled_principal_point_offset = (
            self.film().size()
            * self.principal_point_offset
            / mi.Vector2f(self.film().crop_size())
        )

        near_p = self.sample_to_camera @ mi.Point3f(
            position_sample.x + scaled_principal_point_offset.x,
            position_sample.y + scaled_principal_point_offset.y,
            0.0,
        )
        d = dr.normalize(mi.Vector3d(near_p))

        print("position_sample", position_sample.shape)

        data_tensor_size = self.block_cb.cb_size
        data_tensor = torch.zeros(data_tensor_size, device="cuda", dtype=torch.float32)
        data_tensor[0] = 36
        data_tensor[1] = 1
        data_tensor[2] = unpack("f", pack("i", 10))[0]
        data_tensor[3] = unpack("f", pack("i", 10))[0]
        data_tensor[4] = 10

        for i, p in enumerate(self.block_cb.parameters):
            data_tensor[i] = p
        print("data_tensor", data_tensor)
        # rays = torch.zeros([data_tensor[1], 11])

        # self.run_shader(
        #     random=position_sample.torch(),
        #     data_tensor=data_tensor,
        #     rays=rays,
        #     pixel_targets=position_sample.torch(),
        # )

        ray.o = self.to_world.translation()
        ray.d = self.to_world @ mi.Vector3d(d)

        inv_z = dr.rcp(d.z)
        near_t = self.near_clip * inv_z
        far_t = self.far_clip * inv_z
        ray.o += ray.d * near_t
        ray.maxt = far_t - near_t

        return ray, wav_weight

    # def sample_ray_differential(
    #     self, time, wavelength_sample, position_sample, aperture_sample, active=True
    # ):
    #     print("sample_ray_differential")
    #     wavelengths, wav_weight = self.sample_wavelengths(
    #         dr.zeros(mi.SurfaceInteraction3f), wavelength_sample, active
    #     )
    #     ray = mi.RayDifferential3f()
    #     ray.time = time
    #     ray.wavelengths = wavelengths

    #     scaled_principal_point_offset = (
    #         self.film().size()
    #         * self.principal_point_offset
    #         / mi.Vector2f(self.film().crop_size())
    #     )

    #     near_p = self.sample_to_camera @ mi.Point3f(
    #         position_sample.x + scaled_principal_point_offset.x,
    #         position_sample.y + scaled_principal_point_offset.y,
    #         0.0,
    #     )
    #     d = dr.normalize(mi.Vector3d(near_p))

    #     ray.o = self.to_world.translation()
    #     ray.d = self.to_world @ d

    #     inv_z = dr.rcp(d.z)
    #     near_t = self.near_clip * inv_z
    #     far_t = self.far_clip * inv_z
    #     ray.o += ray.d * near_t
    #     ray.maxt = far_t - near_t

    #     ray.o_x = ray.o_y = ray.o
    #     ray.d_x = self.to_world @ dr.normalize(mi.Vector3d(near_p) + self.dx)
    #     ray.d_y = self.to_world @ dr.normalize(mi.Vector3d(near_p) + self.dy)
    #     ray.has_differentials = True

    #     return ray, wav_weight

    def sample_direction(self, it, sample, active=True):
        trafo = self.to_world
        ref_p = trafo.inverse().transform_affine(it.p)

        ds = mi.DirectionSample3f()
        ds.pdf = 0.0
        active &= (ref_p.z >= self.near_clip) & (ref_p.z <= self.far_clip)
        if dr.none_or_false(active):
            return ds, dr.zeros(mi.Spectrum)

        scaled_principal_point_offset = (
            self.film().size() * self.principal_point_offset / self.film().crop_size()
        )

        screen_sample = self.camera_to_sample @ ref_p
        ds.uv = mi.Point2f(
            screen_sample.x - scaled_principal_point_offset.x,
            screen_sample.y - scaled_principal_point_offset.y,
        )
        active &= (ds.uv.x >= 0) & (ds.uv.x <= 1) & (ds.uv.y >= 0) & (ds.uv.y <= 1)
        if dr.none_or_false(active):
            return ds, dr.zeros(mi.Spectrum)

        ds.uv *= self.resolution

        local_d = mi.Vector3d(ref_p)
        dist = dr.norm(local_d)
        inv_dist = dr.rcp(dist)
        local_d *= inv_dist

        ds.p = trafo.transform_affine(mi.Point3f(0.0))
        ds.d = (ds.p - it.p) * inv_dist
        ds.dist = dist
        ds.n = trafo @ mi.Vector3d(0.0, 0.0, 1.0)
        ds.pdf = dr.select(active, 1.0, 0.0)

        return ds, mi.Spectrum(self.importance(local_d) * inv_dist * inv_dist)

    def bbox(self):
        p = self.to_world @ mi.Point3f(0.0)
        return mi.BoundingBox3f(p, p)

    def importance(self, d):
        ct = mi.Frame3f.cos_theta(d)
        inv_ct = dr.rcp(ct)

        p = mi.Point2f(d.x * inv_ct, d.y * inv_ct)
        valid = (ct > 0) & self.image_rect.contains(p)

        return dr.select(valid, self.normalization * inv_ct * inv_ct * inv_ct, 0.0)

    def to_string(self):
        return f"LensCamera[\n  x_fov = {self.x_fov},\n  near_clip = {self.near_clip},\n  far_clip = {self.far_clip},\n  film = {self.film()},\n  sampler = {self.sampler()},\n  resolution = {self.resolution},\n  shutter_open = {self.shutter_open()},\n  shutter_open_time = {self.shutter_open_time()},\n  to_world = {self.to_world}\n]"


mi.register_sensor("physical_lens_cam", lambda props: LensCamera(props))
