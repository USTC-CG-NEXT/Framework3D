#include "GUI/window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "nodes/core/def/node_def.hpp"
#include "polyscope/polyscope.h"
#include "polyscope_widget/polyscope_widget_payload.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_click_pos)
{
    b.add_output<glm::vec2>("Click Position");
}

NODE_EXECUTION_FUNCTION(get_click_pos)
{
    auto global_payload = params.get_global_payload<PolyscopeWidgetPayload>();

    std::string renderer_window_name = global_payload.renderer_window_name;
    ImGuiWindow* window = ImGui::FindWindowByName(renderer_window_name.c_str());
    ImGuiIO& io = ImGui::GetIO();

    if (window == nullptr) {
        return false;
    }

    while (!io.MouseClicked[0]) {
        // Wait for the mouse click
    }

    auto click_pos = io.MouseClickedPos[0];
    // Compute the click position in the window
    auto window_pos = window->Pos;
    auto window_size = window->Size;
    auto click_x = click_pos.x - window_pos.x;
    auto click_y = click_pos.y - window_pos.y;
    auto click_pos_in_window = glm::vec2{ click_x, click_y };

    params.set_output("Click Position", click_pos_in_window);

    return true;
}

NODE_DECLARATION_UI(get_click_pos);
NODE_DEF_CLOSE_SCOPE