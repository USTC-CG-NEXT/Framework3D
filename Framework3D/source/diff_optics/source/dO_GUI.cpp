#include <GUI/widget.h>
#include <diff_optics/api.h>

#include <memory>

USTC_CG_NAMESPACE_OPEN_SCOPE
class DiffOpticsGUI : public IWidget {
   public:
    bool BuildUI() override;
};

bool DiffOpticsGUI::BuildUI()
{
    ImGui::Begin("123");
    ImGui::Text("Hello, world!");

    DrawCircle({ 80, 80 }, 10);

    DrawArc({ 100, 100 }, 50, 0, 3.14, 2, ImColor(255, 0, 0, 255), 100);

    DrawFunction([](float x) { return 0.1*x * x; }, { -30, 30 }, { 300, 300 });

    ImGui::End();
    // ImGui::ShowDemoWindow();
    return true;
}

std::unique_ptr<IWidget> createDiffOpticsGUI()
{
    return std::make_unique<DiffOpticsGUI>();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
