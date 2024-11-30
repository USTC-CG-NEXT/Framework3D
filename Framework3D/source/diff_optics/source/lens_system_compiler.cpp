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
    std::string functions = sphere_intersection + "\n";
    functions += flat_intersection + "\n";
    functions += occluder_intersection + "\n";
    functions += get_relative_refractive_index + "\n";
    std::string const_buffer;

    indent += 4;

    const_buffer += "struct LensSystemData\n{\n";
    const_buffer += emit_line("float2 film_size;", 2);
    const_buffer += emit_line("int2 film_resolution;", 2);
    const_buffer += emit_line("float film_distance;", 1);

    std::string raygen_shader =
        "[Differentiable]\n RayInfo raygen(float2 pixel_id, inout float3 "
        "weight, "
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
    raygen_shader += indent_str(indent) + "RayInfo ray;\n";
    raygen_shader += indent_str(indent) + "RayInfo next_ray;\n";
    raygen_shader += emit_line("float t = 0");

    // Sample origin on the film
    raygen_shader += indent_str(indent) +
                     "float2 film_pos = -((0.5f+float2(pixel_id)) / "
                     "float2(data.film_resolution)-0.5f) * "
                     "data.film_size;\n";

    raygen_shader += indent_str(indent) +
                     "ray.Origin = float3(detach(film_pos), "
                     "-data.film_distance);\n";

    // TMin and TMax

    raygen_shader += indent_str(indent) + "ray.TMin = 0;\n";
    raygen_shader += indent_str(indent) + "ray.TMax = 1000;\n";

    raygen_shader += emit_line("next_ray = ray;");

    raygen_shader += indent_str(indent) + "weight = 1;\n";

    CompiledDataBlock block;

    for (auto lens_layer : lens_system->lenses) {
        block.parameter_offsets[id] = cb_size;
        lens_layer->EmitShader(
            id,
            const_buffer,
            raygen_shader,
            get_lens_data_from_torch_tensor,
            this);

        if (require_ray_visualization) {
            raygen_shader += emit_line(
                "ray_visualization_" + std::to_string(id) +
                "[pixel_id.y * data.film_resolution.x + "
                "pixel_id.x] "
                "= RayInfo(ray.Origin, 0, ray.Direction, t);");
        }
        raygen_shader += emit_line("ray = next_ray;");
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
            const_buffer += emit_line(
                "RWStructuredBuffer<RayInfo> ray_visualization_" +
                std::to_string(i));
        }
    }

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "return ray;\n";
    raygen_shader += "}";

    get_lens_data_from_torch_tensor += emit_line("return data;");
    get_lens_data_from_torch_tensor += "}";

    indent -= 4;

    auto final_shader = header + functions + const_buffer + raygen_shader +
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

USTC_CG_NAMESPACE_CLOSE_SCOPE