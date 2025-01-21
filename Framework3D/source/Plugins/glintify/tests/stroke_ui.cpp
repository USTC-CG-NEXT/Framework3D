#include <GUI/widget.h>
#include <GUI/window.h>
#include <gtest/gtest.h>

#include "RHI/rhi.hpp"
#include "glintify/glintify.hpp"
#include "glintify/mesh.hpp"

#define TEST_VIRTUAL_POINT 0

class StrokeEditWidget : public USTC_CG::IWidget {
   public:
    StrokeEditWidget(std::shared_ptr<USTC_CG::StrokeSystem> stroke_system)
        : stroke_system(stroke_system)
    {
        stroke_system->set_camera_position(camera_position);
        stroke_system->set_light_position(light_position);
        stroke_system->set_camera_move_range(camera_move_range);
    }
    ~StrokeEditWidget() override = default;

    bool BuildUI() override
    {
        if (ImGui::SliderFloat3(
                "Camera Position", &camera_position.x, -10.0f, 10.0f)) {
            stroke_system->set_camera_position(camera_position);
        }

        if (ImGui::SliderFloat3(
                "Light Position", &light_position.x, -10.0f, 10.0f)) {
            stroke_system->set_light_position(light_position);
        }

        if (ImGui::SliderFloat2(
                "Camera Move Range", &camera_move_range.x, -10.0f, 10.0f)) {
            stroke_system->set_camera_move_range(camera_move_range);
        }

#if TEST_VIRTUAL_POINT
        if (ImGui::SliderFloat3(
                "Virtual Point Position",
                &virtual_point_position.x,
                -1.0f,
                1.0f)) {
            stroke_system->clear();
            stroke_system->add_virtual_point(virtual_point_position);
        }
#endif

        if (ImGui::Checkbox(
                "Consider Occlusion", &fill_ranges_with_occlusion)) {
            stroke_system->is_dirty = true;
        }
        stroke_system->fill_ranges(fill_ranges_with_occlusion);

        if (ImGui::Button("Save")) {
            auto end_points = stroke_system->get_all_endpoints();
            std::ofstream file("stroke.txt");
            file << "[";
            for (auto& line : end_points) {
                file << "[";
                for (auto& point : line) {
                    file << "[" << point.x << ", " << point.y << "],";
                }
                file << "],";
            }
            file << "]";
        }

        return true;
    }

   protected:
    std::string GetWindowUniqueName() override
    {
        return "Stroke Edit";
    }

   private:
    bool fill_ranges_with_occlusion = true;

    glm::vec3 camera_position = glm::vec3(0, 1.0, -6);
    glm::vec3 light_position = glm::vec3(0, 6, -4);
    glm::vec2 camera_move_range = glm::vec2(-3.0, 3.0);

    glm::vec3 virtual_point_position = glm::vec3(0, 0, -1);

    std::shared_ptr<USTC_CG::StrokeSystem> stroke_system;
};

class StrokeVisualizeWidget : public USTC_CG::IWidget {
   public:
    StrokeVisualizeWidget(std::shared_ptr<USTC_CG::StrokeSystem> stroke_system)
        : stroke_system(stroke_system)
    {
    }
    ~StrokeVisualizeWidget() override = default;

   protected:
    std::string GetWindowUniqueName() override
    {
        return "Stroke Visualize";
    }

   public:
    bool BuildUI() override
    {
        stroke_system->calc_scratches();

        auto lines = stroke_system->get_all_endpoints();

        float scale = std::min(width, height);

        for (auto& line : lines) {
            if (!line.empty())
                for (int i = 0; i < line.size() - 1; ++i) {
                    DrawLine(
                        ImVec2(scale * line[i].x, scale * (1.0f - line[i].y)),
                        ImVec2(
                            scale * line[i + 1].x,
                            scale * (1.0f - line[i + 1].y)),
                        1.0f,
                        IM_COL32(255, 255, 255, 255));
                }
        }
        return true;
    }

   private:
    std::shared_ptr<USTC_CG::StrokeSystem> stroke_system;
};

int main()
{
    using namespace USTC_CG;

    auto stroke_system = std::make_shared<StrokeSystem>();
    auto mesh = USTC_CG::Mesh::load_from_obj("rings.obj");

    auto triangulated = mesh.get_triangulated_mesh();
    auto edge_samples = mesh.sample_on_edges(0.099f);

    for (auto& sample : edge_samples) {
        stroke_system->add_virtual_point(sample);
    }

    stroke_system->set_occlusion(triangulated.vertices, triangulated.indices);

    // stroke_system->add_virtual_point({ 0, 0, -1 });

    Window window;

    std::unique_ptr<IWidget> stroke_edit_widget =
        std::make_unique<StrokeEditWidget>(stroke_system);

    std::unique_ptr<IWidget> stroke_visualize_widget =
        std::make_unique<StrokeVisualizeWidget>(stroke_system);

    window.register_widget(std::move(stroke_edit_widget));
    window.register_widget(std::move(stroke_visualize_widget));
    window.run();
    return 0;
}