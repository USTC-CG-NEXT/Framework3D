#define IMGUI_DEFINE_MATH_OPERATORS

#include "diff_optics/lens_system.hpp"

#include <fstream>
#include <memory>
#include <vector>

#include "RHI/ShaderFactory/shader.hpp"
#include "RHI/ShaderFactory/shader_reflection.hpp"
#include "dO_GUI.hpp"
#include "diff_optics/lens_system_compiler.hpp"
#include "imgui.h"
#include "lens_system_gui.hpp"
#include "optical_material.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
int div_ceil(int dividend, int divisor)
{
    return (dividend + (divisor - 1)) / divisor;
}

LensLayer::LensLayer(float center_x, float center_y)
    : center_pos(center_x, center_y)
{
}

LensLayer::~LensLayer() = default;

void LensLayer::set_axis(float axis_pos)
{
    center_pos[1] = axis_pos;
}

void LensLayer::set_pos(float x)
{
    center_pos[0] = x;
}

NullLayer::NullLayer(float center_x, float center_y)
    : LensLayer(center_x, center_y)
{
    painter = std::make_unique<NullPainter>();
    compiler = std::make_unique<NullCompiler>(this);
}

Occluder::Occluder(float radius, float x, float y)
    : radius(radius),
      LensLayer(x, y)
{
    painter = std::make_unique<OccluderPainter>();
    compiler = std::make_unique<OccluderCompiler>(this);
}

void SphericalLens::update_info(float center_x, float center_y)
{
    theta_range = abs(asin(diameter / (2 * radius_of_curvature)));
    sphere_center = { center_x + radius_of_curvature, center_y };
}

SphericalLens::SphericalLens(float d, float roc, float center_x, float center_y)
    : LensLayer(center_x, center_y),
      diameter(d),
      radius_of_curvature(roc)
{
    update_info(center_x, center_y);
    painter = std::make_unique<SphericalLensPainter>();
    compiler = std::make_unique<SphericalLensCompiler>(this);
}

FlatLens::FlatLens(float d, float center_x, float center_y)
    : LensLayer(center_x, center_y),
      diameter(d)
{
    painter = std::make_unique<FlatLensPainter>();
    compiler = std::make_unique<FlatLensCompiler>(this);
}

void FlatLens::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    diameter = j["diameter"];
}

void FlatLens::fill_block_data(float* ptr)
{
    ptr[0] = diameter;
    ptr[1] = center_pos[0];
    ptr[2] = optical_property.refractive_index;
    ptr[3] = optical_property.abbe_number;
}

Sensor::Sensor(float d, float center_x, float center_y)
    : LensLayer(center_x, center_y)
{
    diameter = d;
    painter = std::make_unique<SensorPainter>();
}
void Sensor::deserialize(const nlohmann::json& j)
{
}

void Sensor::fill_block_data(float* ptr)
{
}

LensSystem::LensSystem() : gui(std::make_unique<LensSystemGUI>(this))
{
    block = std::make_unique<CompiledDataBlock>();
}

void LensSystem::add_lens(std::shared_ptr<LensLayer> lens)
{
    lenses.push_back(lens);
}

void LensLayer::deserialize(const nlohmann::json& j)
{
    optical_property = get_optical_property(j["material"]);
}

void NullLayer::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
}

void NullLayer::fill_block_data(float* ptr)
{
    ptr[0] = optical_property.refractive_index;
}

void Occluder::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    radius = j["diameter"].get<float>() / 2.0f;
}

void SphericalLens::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    diameter = j["diameter"];
    radius_of_curvature = j["roc"];
    theta_range = abs(atan(diameter / (2 * radius_of_curvature)));
    sphere_center = { center_pos[0] + radius_of_curvature, center_pos[1] };
    // if (j.contains("additional_params")) {
    //     high_order_polynomial_coefficients =
    //         j["additional_params"].get<std::vector<float>>();
    // }
}

void LensSystem::deserialize(const std::string& json)
{
    nlohmann::json j = nlohmann::json::parse(json);
    float accumulated_distance = 0.0f;
    for (const auto& item : j["data"]) {
        std::shared_ptr<LensLayer> layer;
        accumulated_distance += item["distance"].get<float>();
        if (item["type"] == "O") {
            layer = std::make_shared<NullLayer>(accumulated_distance, 0.0f);
        }
        else if (item["type"] == "A") {
            layer = std::make_shared<Occluder>(
                item["diameter"].get<float>() / 2.0f,
                accumulated_distance,
                0.0f);
        }
        else if (item["type"] == "S" && item["roc"].get<float>() != 0) {
            layer = std::make_shared<SphericalLens>(
                item["diameter"].get<float>(),
                item["roc"],
                accumulated_distance,
                0.0f);
        }
        else if (item["type"] == "S" && item["roc"].get<float>() == 0) {
            layer = std::make_shared<FlatLens>(
                item["diameter"].get<float>(), accumulated_distance, 0.0f);
        }

        layer->deserialize(item);
        add_lens(layer);
    }
}

void LensSystem::deserialize(const std::filesystem::path& path)
{
    std::ifstream json_file(path);
    std::string json(
        (std::istreambuf_iterator<char>(json_file)),
        std::istreambuf_iterator<char>());
    deserialize(json);
}

const char* double_gauss = R"(
{
    "Originate": "US02532751-1",
    "data": [
        {
            "type": "O",
            "distance": 0.0,
            "roc": 0.0,
            "diameter": 0.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0,
            "roc": 13.354729316461547,
            "diameter": 17.4,
            "material": "SSK4",
            "additional_params": [0.005, 1e-6, 1e-8, -3e-10]
        },
        {
            "type": "S",
            "distance": 2.2352,
            "roc": 35.64148197667863,
            "diameter": 17.4,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 10.330017837998932,
            "diameter": 14.0,
            "material": "SK1"
        },
        {
            "type": "S",
            "distance": 3.1750,
            "roc": 0.0,
            "diameter": 14.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 6.494496063151893,
            "diameter": 9.0,
            "material": "VACUUM"
        },
        {
            "type": "A",
            "distance": 3.8608,
            "roc": 0.0,
            "diameter": 4.886,
            "material": "OCCLUDER"
        },
        {
            "type": "S",
            "distance": 3.302,
            "roc": -7.026950339915501,
            "diameter": 9.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 0.0,
            "diameter": 12.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 2.7686,
            "roc": -9.746574604143909,
            "diameter": 12.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 69.81692521236866,
            "diameter": 14.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 1.7526,
            "roc": -19.226275376106166,
            "diameter": 14.0,
            "material": "VACUUM"
        }
    ]
}

)";

void LensSystem::set_default()
{
    deserialize(std::string(double_gauss));
    auto sensor = std::make_shared<Sensor>(35.f, 36.0f, 0.0f);
    add_lens(sensor);
}

std::vector<std::vector<RayInfo>> LensSystem::trace_ray(
    const std::vector<RayInfo>& ray_in)
{
    if (!ray_trace_func) {
        compile_ray_trace_func();
    }
    return ray_trace_func(ray_in);
}

struct UniformState {
    void* data;
    CPPPrelude::RWStructuredBuffer<RayInfo> rays[20];
};

void LensSystem::compile_ray_trace_func()
{
    if (!compiler) {
        compiler = std::make_unique<LensSystemCompiler>();
        std::string shader;
        std::tie(shader, *block) = compiler->compile(this, true);

        std::ofstream file("lens_shader_cpu.slang");
        file << shader;
        file.close();
    }

    ShaderFactory shader_factory;
    shader_factory.set_search_path(RENDERER_SHADER_DIR);

    ShaderReflectionInfo reflection;
    std::string error_string;
    auto program_handle = shader_factory.compile_cpu_executable(
        "ray_trace_main",
        nvrhi::ShaderType::Compute,
        "shaders/physical_lens_raygen_cpu.slang",
        reflection,
        error_string);

    ray_trace_func = [program_handle,
                      this](const std::vector<RayInfo>& ray_in) {
        auto rays = ray_in;

        auto trancient_rays_count = lenses.size();

        compiler->fill_block_data(this, *block);

        UniformState state;
        state.data = (void*)(block->parameters.data());

        std::vector<CPPPrelude::RWStructuredBuffer<RayInfo>> ray_buffers;

        ray_buffers.resize(trancient_rays_count + 1);
        ray_buffers[0].data = rays.data();
        ray_buffers[0].count = rays.size();

        std::vector<std::vector<RayInfo>> ray_visualizations;

        for (size_t i = 0; i < trancient_rays_count; i++) {
            ray_visualizations.push_back(std::vector<RayInfo>(rays.size()));
            ray_buffers[i + 1].data = ray_visualizations[i].data();
            ray_buffers[i + 1].count = ray_visualizations[i].size();
        }

        memcpy(
            state.rays,
            ray_buffers.data(),
            sizeof(CPPPrelude::RWStructuredBuffer<RayInfo>) *
                ray_buffers.size());

        CPPPrelude::ComputeVaryingInput input;
        input.startGroupID = { 0, 0, 0 };
        input.endGroupID = { unsigned(div_ceil(rays.size(), 128)), 1, 1 };
        program_handle->host_call(input, state);

        ray_visualizations[0] = rays;
        return ray_visualizations;
    };
}

static const std::string sphere_raygen_template = R"(
    RayInfo
)";

void Occluder::fill_block_data(float* ptr)
{
    ptr[0] = radius;
    ptr[1] = center_pos[0];
    ptr[2] = optical_property.refractive_index;
}

void SphericalLens::fill_block_data(float* ptr)
{
    ptr[0] = diameter;
    ptr[1] = radius_of_curvature;
    ptr[2] = theta_range;
    ptr[3] = sphere_center[0];
    ptr[4] = center_pos[0];
    ptr[5] = optical_property.refractive_index;
    ptr[6] = optical_property.abbe_number;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
