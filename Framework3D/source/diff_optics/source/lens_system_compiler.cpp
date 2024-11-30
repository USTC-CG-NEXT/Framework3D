#include "diff_optics/lens_system_compiler.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
const std::string LensSystemCompiler::sphere_intersection = R"(
import lens.intersect_sphere;
)";

const std::string LensSystemCompiler::flat_intersection = R"(
import lens.intersect_flat;
)";

const std::string LensSystemCompiler::occluder_intersection = R"(
import lens.intersect_occluder;
)";

const std::string get_relative_refractive_index = R"(
import lens.lens_utils;
)";

std::string LensSystemCompiler::emit_line(
    const std::string& line,
    unsigned cb_size_occupied)
{
    cb_size += cb_size_occupied;
    return indent_str(indent) + line + ";\n";
}

std::tuple<std::string, CompiledDataBlock> LensSystemCompiler::compile(
    LensSystem* lens_system,
    bool require_ray_visualization)
{
    cb_size = 0;

    std::string header = R"(
#include "utils/random.slangh"
#include "utils/Math/MathConstants.slangh"
import utils.ray;
import Utils.Math.MathHelpers;
)";
    std::string functions = sphere_intersection;
    functions += flat_intersection;
    functions += occluder_intersection;
    functions += get_relative_refractive_index;
    std::string const_buffer;

    indent += 4;

    const_buffer += "struct LensSystemData\n{\n";
    const_buffer += emit_line("float2 film_size;", 2);
    const_buffer += emit_line("int2 film_resolution;", 2);
    const_buffer += emit_line("float film_distance;", 1);

    std::string sample_dir_shader = R"(
[Differentiable]
RayInfo sample_dir(float2 pixel_id, float2 seed2, LensSystemData data)
{
)";

    std::string ray_trace_shader = R"(
[Differentiable]
RayInfo ray_trace(RayInfo ray, LensSystemData data)
{
)";

    std::string raygen_shader =
        "[Differentiable]\n RayInfo raygen(float2 pixel_id, "
        "in float2 "
        "seed2, LensSystemData data)\n{";

    std::string get_lens_data_from_torch_tensor = "[Differentiable]";
    get_lens_data_from_torch_tensor +=
        "LensSystemData get_lens_data_from_torch_tensor(DiffTensorView "
        "tensor)\n{";
    get_lens_data_from_torch_tensor += emit_line("LensSystemData data;");

    get_lens_data_from_torch_tensor +=
        emit_line("data.film_size.x = tensor[uint(0)]");
    get_lens_data_from_torch_tensor +=
        emit_line("data.film_size.y = tensor[uint(1)]");
    get_lens_data_from_torch_tensor +=
        emit_line("data.film_resolution.x = reinterpret<int>(tensor[uint(2)])");
    get_lens_data_from_torch_tensor +=
        emit_line("data.film_resolution.y = reinterpret<int>(tensor[uint(3)])");
    get_lens_data_from_torch_tensor +=
        emit_line("data.film_distance = tensor[uint(4)]");

    int id = 0;

    raygen_shader += "\n";
    sample_dir_shader +=
        emit_line("RayInfo ray = sample_dir(pixel_id, seed2, data);");
    sample_dir_shader += emit_line("return ray_trace(ray, data);");

    sample_dir_shader += emit_line("float t = 0");

    // Sample origin on the film
    sample_dir_shader += indent_str(indent) +
                         "float2 film_pos = -((0.5f+float2(pixel_id)) / "
                         "float2(data.film_resolution)-0.5f) * "
                         "data.film_size;\n";

    sample_dir_shader += indent_str(indent) +
                         "ray.Origin = float3(detach(film_pos), "
                         "-data.film_distance);\n";

    // TMin and TMax

    sample_dir_shader += indent_str(indent) + "ray.TMin = 0;\n";
    sample_dir_shader += indent_str(indent) + "ray.TMax = 1000;\n";

    ray_trace_shader += indent_str(indent) + "RayInfo next_ray = ray;\n";

    CompiledDataBlock block;

    for (int i = 0; i < lens_system->lenses.size(); i++) {
        auto lens_layer = lens_system->lenses[i];

        block.parameter_offsets[id] = cb_size;

        lens_layer->compiler->EmitCbDataLoad(
            id, const_buffer, get_lens_data_from_torch_tensor, this);
        lens_layer->compiler->EmitRayTrace(id, ray_trace_shader, this);
        if (i == 1) {
            lens_layer->compiler->EmitSampleDirFromSensor(
                id, sample_dir_shader, this);
        }

        if (require_ray_visualization) {
            ray_trace_shader += emit_line(
                "ray_visualization_" + std::to_string(id) +
                "[pixel_id.y * data.film_resolution.x + "
                "pixel_id.x] "
                "= RayInfo(ray.Origin, 0, ray.Direction, t)");
        }
        ray_trace_shader += emit_line("ray = next_ray;");
        id++;
    }

    block.parameters.resize(cb_size);
    block.cb_size = cb_size;

    for (int i = 0; i < lens_system->lenses.size(); ++i) {
        auto lens = lens_system->lenses[i];

        auto offset = block.parameter_offsets[i];

        lens->fill_block_data(block.parameters.data() + offset);
    }

    const_buffer += "};\n";

    if (require_ray_visualization) {
        for (size_t i = 0; i < lens_system->lenses.size(); i++) {
            const_buffer += "RWStructuredBuffer<RayInfo> ray_visualization_" +
                            std::to_string(i) + ";\n";
        }
    }

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "return ray;\n";
    raygen_shader += "}";

    get_lens_data_from_torch_tensor += emit_line("return data;");
    get_lens_data_from_torch_tensor += "}";

    ray_trace_shader += "}\n";

    sample_dir_shader += emit_line("return ray");
    sample_dir_shader += "}\n";

    indent -= 4;

    auto final_shader = header + functions + const_buffer + sample_dir_shader +
                        ray_trace_shader + raygen_shader +
                        get_lens_data_from_torch_tensor;

    return std::make_tuple(final_shader, block);
}

void LensSystemCompiler::fill_block_data(
    LensSystem* lens_system,
    CompiledDataBlock& data_block)
{
    for (int i = 0; i < lens_system->lenses.size(); ++i) {
        auto lens = lens_system->lenses[i];

        auto offset = data_block.parameter_offsets[i];

        lens->fill_block_data(data_block.parameters.data() + offset);
    }
}

void NullCompiler::EmitCbDataLoad(
    int id,
    std::string& constant_buffer,
    std::string& data_load,
    LensSystemCompiler* compiler)
{
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_refractive_index");
}

void NullCompiler::EmitRayTrace(
    int id,
    std::string& execution,
    LensSystemCompiler* compiler)
{
}

void NullCompiler::EmitSampleDirFromSensor(
    int id,
    std::string& sample_from_sensor,
    LensSystemCompiler* compiler)
{
}

void OccluderCompiler::EmitCbDataLoad(
    int id,
    std::string& constant_buffer,
    std::string& data_load,
    LensSystemCompiler* compiler)
{
    add_cb_data_load(id, constant_buffer, data_load, compiler, "radius");
    add_cb_data_load(id, constant_buffer, data_load, compiler, "center_pos");
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_refractive_index");
}

void OccluderCompiler::EmitRayTrace(
    int id,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    execution += compiler->emit_line(
        std::string("next_ray = intersect_occluder(ray, t, ") + "data.radius_" +
        std::to_string(id) + ", " + "data.center_pos_" + std::to_string(id) +
        ")");
}

void OccluderCompiler::EmitSampleDirFromSensor(
    int id,
    std::string& sample_from_sensor,
    LensSystemCompiler* compiler)
{
    // Not implemented
    throw std::runtime_error("Not implemented");
}

void SphericalLensCompiler::EmitCbDataLoad(
    int id,
    std::string& constant_buffer,
    std::string& data_load,
    LensSystemCompiler* compiler)
{
    add_cb_data_load(id, constant_buffer, data_load, compiler, "diameter");
    add_cb_data_load(
        id, constant_buffer, data_load, compiler, "radius_of_curvature");
    add_cb_data_load(id, constant_buffer, data_load, compiler, "theta_range");
    add_cb_data_load(id, constant_buffer, data_load, compiler, "sphere_center");
    add_cb_data_load(id, constant_buffer, data_load, compiler, "center_pos");
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_refractive_index");
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_abbe_number");
}

void SphericalLensCompiler::EmitRayTrace(
    int id,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    execution += compiler->emit_line(
        std::string("float relative_refractive_index_") + std::to_string(id) +
        " = get_relative_refractive_index(" +
        "data.optical_property_refractive_index_" + std::to_string(id - 1) +
        ", " + "data.optical_property_refractive_index_" + std::to_string(id) +
        ")");

    execution += compiler->emit_line(
        std::string("next_ray =  intersect_sphere(ray, t, ") +
        "data.radius_of_curvature_" + std::to_string(id) + ", " +
        "data.sphere_center_" + std::to_string(id) + ", " +
        "data.theta_range_" + std::to_string(id) + ", " +
        "relative_refractive_index_" + std::to_string(id) + ", " +
        "data.optical_property_abbe_number_" + std::to_string(id) + ")");
}

void SphericalLensCompiler::EmitSampleDirFromSensor(
    int id,
    std::string& sample_from_sensor,
    LensSystemCompiler* compiler)
{
    sample_from_sensor += compiler->emit_line(
        std::string("float2 target_pos = sample_disk(seed2) * ") +
        "data.diameter_" + std::to_string(id) + "/2.0f");

    sample_from_sensor += compiler->emit_line(
        "float3 sampled_point_" + std::to_string(id) +
        " = float3(target_pos.x, target_pos.y, " + "data.center_pos_" +
        std::to_string(id) + ")");

    sample_from_sensor += compiler->emit_line(
        "ray.Direction = normalize(sampled_point_" + std::to_string(id) +
        " - ray.Origin)");
}

void FlatLensCompiler::EmitCbDataLoad(
    int id,
    std::string& constant_buffer,
    std::string& data_load,
    LensSystemCompiler* compiler)
{
    add_cb_data_load(id, constant_buffer, data_load, compiler, "diameter");
    add_cb_data_load(id, constant_buffer, data_load, compiler, "center_pos");
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_refractive_index");
    add_cb_data_load(
        id,
        constant_buffer,
        data_load,
        compiler,
        "optical_property_abbe_number");
}

void FlatLensCompiler::EmitRayTrace(
    int id,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    execution += compiler->emit_line(
        std::string("float relative_refractive_index_") + std::to_string(id) +
        " = get_relative_refractive_index(" +
        "data.optical_property_refractive_index_" + std::to_string(id - 1) +
        ", " + "data.optical_property_refractive_index_" + std::to_string(id) +
        ")");

    execution += compiler->emit_line(
        "next_ray = intersect_flat(ray, t, data.diameter_" +
        std::to_string(id) + ", data.center_pos_" + std::to_string(id) + ", " +
        "relative_refractive_index_" + std::to_string(id) + ", " +
        "data.optical_property_abbe_number_" + std::to_string(id) + ")");
}

void FlatLensCompiler::EmitSampleDirFromSensor(
    int id,
    std::string& sample_from_sensor,
    LensSystemCompiler* compiler)
{  // Not implemented
    throw std::runtime_error("Not implemented");
}

USTC_CG_NAMESPACE_CLOSE_SCOPE