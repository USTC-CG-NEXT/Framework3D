import json
import numpy as np
from typing import List, Dict, Any


class OpticalProperty:
    def __init__(self, refractive_index: float, abbe_number: float):
        self.refractive_index = refractive_index
        self.abbe_number = abbe_number


def get_optical_property(name: str) -> OpticalProperty:
    name = name.upper()
    properties = {
        "VACUUM": OpticalProperty(1.0, float("inf")),
        "AIR": OpticalProperty(1.000293, float("inf")),
        "OCCLUDER": OpticalProperty(1.0, float("inf")),
        "F2": OpticalProperty(1.620, 36.37),
        "F15": OpticalProperty(1.60570, 37.831),
        "UVFS": OpticalProperty(1.458, 67.82),
        "BK10": OpticalProperty(1.49780, 66.954),
        "N-BAF10": OpticalProperty(1.67003, 47.11),
        "N-BK7": OpticalProperty(1.51680, 64.17),
        "N-SF1": OpticalProperty(1.71736, 29.62),
        "N-SF2": OpticalProperty(1.64769, 33.82),
        "N-SF4": OpticalProperty(1.75513, 27.38),
        "N-SF5": OpticalProperty(1.67271, 32.25),
        "N-SF6": OpticalProperty(1.80518, 25.36),
        "N-SF6HT": OpticalProperty(1.80518, 25.36),
        "N-SF8": OpticalProperty(1.68894, 31.31),
        "N-SF10": OpticalProperty(1.72828, 28.53),
        "N-SF11": OpticalProperty(1.78472, 25.68),
        "SF1": OpticalProperty(1.71736, 29.51),
        "SF2": OpticalProperty(1.64769, 33.85),
        "SF4": OpticalProperty(1.75520, 27.58),
        "SF5": OpticalProperty(1.67270, 32.21),
        "SF6": OpticalProperty(1.80518, 25.43),
        "SF18": OpticalProperty(1.72150, 29.245),
        "BAF10": OpticalProperty(1.67, 47.05),
        "SK1": OpticalProperty(1.61030, 56.712),
        "SK16": OpticalProperty(1.62040, 60.306),
        "SSK4": OpticalProperty(1.61770, 55.116),
        "B270": OpticalProperty(1.52290, 58.50),
        "S-NPH1": OpticalProperty(1.8078, 22.76),
        "D-K59": OpticalProperty(1.5175, 63.50),
        "FLINT": OpticalProperty(1.6200, 36.37),
        "PMMA": OpticalProperty(1.491756, 58.00),
        "POLYCARB": OpticalProperty(1.585470, 30.00),
    }
    return properties.get(name, OpticalProperty(1.0, 0.0))


class LensLayer:
    def __init__(self, center_x: float, center_y: float):
        self.center_pos = [center_x, center_y]
        self.optical_property = OpticalProperty(0.0, 0.0)

    def set_axis(self, axis_pos: float):
        self.center_pos[1] = axis_pos

    def set_pos(self, x: float):
        self.center_pos[0] = x

    def deserialize(self, data: Dict[str, Any]):
        self.optical_property = get_optical_property(data["material"])

    def fill_block_data(self, ptr: List[float]):
        raise NotImplementedError

    def emit_shader(
        self,
        id: int,
        constant_buffer: str,
        execution: str,
        compiler: "LensSystemCompiler",
    ):
        raise NotImplementedError


class NullLayer(LensLayer):
    def __init__(self, center_x: float, center_y: float):
        super().__init__(center_x, center_y)

    def deserialize(self, data: Dict[str, Any]):
        super().deserialize(data)

    def fill_block_data(self, ptr: List[float]):
        ptr[0] = self.optical_property.refractive_index

    def emit_shader(
        self,
        id: int,
        constant_buffer: str,
        execution: str,
        compiler: "LensSystemCompiler",
    ):
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_refractive_index;", 1
        )


class Occluder(LensLayer):
    def __init__(self, radius: float, center_x: float, center_y: float):
        super().__init__(center_x, center_y)
        self.radius = radius

    def deserialize(self, data: Dict[str, Any]):
        super().deserialize(data)
        self.radius = data["diameter"] / 2.0

    def fill_block_data(self, ptr: List[float]):
        ptr[0] = self.radius
        ptr[1] = self.center_pos[0]
        ptr[2] = self.optical_property.refractive_index

    def emit_shader(
        self,
        id: int,
        constant_buffer: str,
        execution: str,
        compiler: "LensSystemCompiler",
    ):
        constant_buffer += compiler.emit_line(f"float radius_{id};", 1)
        constant_buffer += compiler.emit_line(f"float center_pos_{id};", 1)
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_refractive_index;", 1
        )
        execution += compiler.emit_line(
            f"next_ray = intersect_occluder(ray, weight, t, lens_system_data.radius_{id}, lens_system_data.center_pos_{id});"
        )


class SphericalLens(LensLayer):
    def __init__(self, diameter: float, roc: float, center_x: float, center_y: float):
        super().__init__(center_x, center_y)
        self.diameter = diameter
        self.radius_of_curvature = roc
        self.update_info(center_x, center_y)

    def update_info(self, center_x: float, center_y: float):
        self.theta_range = abs(np.asin(self.diameter / (2 * self.radius_of_curvature)))
        self.sphere_center = [center_x + self.radius_of_curvature, center_y]

    def deserialize(self, data: Dict[str, Any]):
        super().deserialize(data)
        self.diameter = data["diameter"]
        self.radius_of_curvature = data["roc"]
        self.update_info(self.center_pos[0], self.center_pos[1])

    def fill_block_data(self, ptr: List[float]):
        ptr[0] = self.diameter
        ptr[1] = self.radius_of_curvature
        ptr[2] = self.theta_range
        ptr[3] = self.sphere_center[0]
        ptr[4] = self.center_pos[0]
        ptr[5] = self.optical_property.refractive_index
        ptr[6] = self.optical_property.abbe_number

    def emit_shader(
        self,
        id: int,
        constant_buffer: str,
        execution: str,
        compiler: "LensSystemCompiler",
    ):
        constant_buffer += compiler.emit_line(f"float diameter_{id};", 1)
        constant_buffer += compiler.emit_line(f"float radius_of_curvature_{id};", 1)
        constant_buffer += compiler.emit_line(f"float theta_range_{id};", 1)
        constant_buffer += compiler.emit_line(f"float sphere_center_{id};", 1)
        constant_buffer += compiler.emit_line(f"float center_pos_{id};", 1)
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_refractive_index;", 1
        )
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_abbe_number;", 1
        )
        execution += compiler.emit_line(
            f"next_ray = intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_{id}, lens_system_data.sphere_center_{id}, lens_system_data.theta_range_{id}, relative_refractive_index_{id}, lens_system_data.optical_property_{id}_abbe_number);"
        )


class FlatLens(LensLayer):
    def __init__(self, diameter: float, center_x: float, center_y: float):
        super().__init__(center_x, center_y)
        self.diameter = diameter

    def deserialize(self, data: Dict[str, Any]):
        super().deserialize(data)
        self.diameter = data["diameter"]

    def fill_block_data(self, ptr: List[float]):
        ptr[0] = self.diameter
        ptr[1] = self.center_pos[0]
        ptr[2] = self.optical_property.refractive_index
        ptr[3] = self.optical_property.abbe_number

    def emit_shader(
        self,
        id: int,
        constant_buffer: str,
        execution: str,
        compiler: "LensSystemCompiler",
    ):
        constant_buffer += compiler.emit_line(f"float diameter_{id};", 1)
        constant_buffer += compiler.emit_line(f"float center_pos_{id};", 1)
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_refractive_index;", 1
        )
        constant_buffer += compiler.emit_line(
            f"float optical_property_{id}_abbe_number;", 1
        )
        execution += compiler.emit_line(
            f"next_ray = intersect_flat(ray, weight, t, lens_system_data.diameter_{id}, lens_system_data.center_pos_{id}, relative_refractive_index_{id}, lens_system_data.optical_property_{id}_abbe_number);"
        )


class LensSystem:
    def __init__(self):
        self.lenses: List[LensLayer] = []

    def add_lens(self, lens: LensLayer):
        self.lenses.append(lens)

    def deserialize(self, json_str: str):
        data = json.loads(json_str)
        accumulated_distance = 0.0
        for item in data["data"]:
            accumulated_distance += item["distance"]
            if item["type"] == "O":
                layer = NullLayer(accumulated_distance, 0.0)
            elif item["type"] == "A":
                layer = Occluder(item["diameter"] / 2.0, accumulated_distance, 0.0)
            elif item["type"] == "S" and item["roc"] != 0:
                layer = SphericalLens(
                    item["diameter"], item["roc"], accumulated_distance, 0.0
                )
            elif item["type"] == "S" and item["roc"] == 0:
                layer = FlatLens(item["diameter"], accumulated_distance, 0.0)
            layer.deserialize(item)
            self.add_lens(layer)

    def set_default(self):
        import os

        file_path = os.path.join(os.path.dirname(__file__), "double_gauss.json")
        with open(file_path, "r") as file:
            double_gauss = file.read()
        self.deserialize(double_gauss)


class CompiledDataBlock:
    def __init__(self):
        self.parameters: List[float] = []
        self.parameter_offsets: Dict[int, int] = {}
        self.cb_size: int = 0

    def __eq__(self, other):
        if not isinstance(other, CompiledDataBlock):
            return NotImplemented
        return (
            self.parameters == other.parameters
            and self.parameter_offsets == other.parameter_offsets
            and self.cb_size == other.cb_size
        )

    def __ne__(self, other):
        return not self.__eq__(other)


class LensSystemCompiler:
    def __init__(self):
        self.indent = 0
        self.cb_size = 0

        # Add the compiler to the LensSystem class
        self.sphere_intersection = "import intersect_sphere;"

        self.flat_intersection = "import intersect_flat;"

        self.occluder_intersection = "import intersect_occluder;"

        self.get_relative_refractive_index = "import lens_utils;"

    @staticmethod
    def emit_line(line: str, cb_size_occupied: int = 0) -> str:
        return line + ";\n"

    @staticmethod
    def indent_str(n: int) -> str:
        return " " * n

    def compile(
        self, lens_system: LensSystem, require_ray_visualization: bool
    ) -> (str, CompiledDataBlock):
        self.cb_size = 0

        header = """
    #include "utils/random.slangh"
    #include "utils/Math/MathConstants.slangh"
    #include "utils/ray.h"
    import Utils.Math.MathHelpers;
    """
        functions = self.sphere_intersection + "\n"
        functions += self.flat_intersection + "\n"
        functions += self.occluder_intersection + "\n"
        functions += self.get_relative_refractive_index + "\n"
        const_buffer = ""

        self.indent += 4

        const_buffer += "struct LensSystemData\n{\n"
        const_buffer += self.emit_line("float2 film_size", 2)
        const_buffer += self.emit_line("int2 film_resolution", 2)
        const_buffer += self.emit_line("float film_distance", 1)

        raygen_shader = (
            "RayInfo raygen(int2 pixel_id, inout float3 weight, inout uint seed)\n{"
        )

        id = 0

        raygen_shader += "\n"
        raygen_shader += self.indent_str(self.indent) + "RayInfo ray;\n"
        raygen_shader += self.indent_str(self.indent) + "RayInfo next_ray;\n"
        raygen_shader += self.emit_line("float t = 0")

        # Sample origin on the film
        raygen_shader += (
            self.indent_str(self.indent)
            + "float2 film_pos = -((0.5f+float2(pixel_id)) / float2(lens_system_data.film_resolution)-0.5f) * lens_system_data.film_size;\n"
        )

        raygen_shader += (
            self.indent_str(self.indent)
            + "ray.Origin = float3(film_pos, -lens_system_data.film_distance);\n"
        )

        # TMin and TMax
        raygen_shader += self.indent_str(self.indent) + "ray.TMin = 0;\n"
        raygen_shader += self.indent_str(self.indent) + "ray.TMax = 1000;\n"

        raygen_shader += self.emit_line("next_ray = ray")

        raygen_shader += self.indent_str(self.indent) + "weight = 1;\n"

        block = CompiledDataBlock()

        for lens_layer in lens_system.lenses:
            block.parameter_offsets[id] = self.cb_size
            lens_layer.emit_shader(id, const_buffer, raygen_shader, self)

            if require_ray_visualization:
                raygen_shader += self.emit_line(
                    f"ray_visualization_{id}[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);"
                )
            raygen_shader += self.emit_line("ray = next_ray")
            id += 1

        block.parameters = [0.0] * self.cb_size
        block.cb_size = self.cb_size

        for i, lens in enumerate(lens_system.lenses):
            offset = block.parameter_offsets[i]
            lens.fill_block_data(block.parameters[offset:])

        const_buffer += "};\n"
        const_buffer += "ConstantBuffer<LensSystemData> lens_system_data;\n"

        if require_ray_visualization:
            for i in range(len(lens_system.lenses)):
                const_buffer += self.emit_line(
                    f"RWStructuredBuffer<RayInfo> ray_visualization_{i}"
                )

        raygen_shader += "\n"
        raygen_shader += self.indent_str(self.indent) + "return ray;\n"
        raygen_shader += "}"

        self.indent -= 4

        final_shader = header + functions + const_buffer + raygen_shader

        return final_shader, block

    @staticmethod
    def fill_block_data(lens_system: LensSystem, data_block: CompiledDataBlock):
        for i, lens in enumerate(lens_system.lenses):
            offset = data_block.parameter_offsets[i]
            lens.fill_block_data(data_block.parameters[offset:])


if __name__ == "__main__":

    # Example usage
    lens_system = LensSystem()
    lens_system.set_default()
    compiler = LensSystemCompiler()
    shader_code, compiled_data = compiler.compile(
        lens_system, require_ray_visualization=True
    )
    print(shader_code)
