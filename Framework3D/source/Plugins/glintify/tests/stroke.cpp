#include <GUI/widget.h>
#include <GUI/window.h>

#include "glintify/glintify.hpp"

class StrokeEditWidget : public USTC_CG::IWidget {
   public:
    StrokeEditWidget(std::shared_ptr<USTC_CG::StrokeSystem> stroke_system)
        : stroke_system(stroke_system)
    {
    }
    ~StrokeEditWidget() override = default;

    bool BuildUI() override
    {
        ImGui::Text("Hello, world!");
        return true;
    }

   private:
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
    const char* GetWindowName() override
    {
        return "Stroke Visualize";
    }

   public:
    bool BuildUI() override
    {
        ImGui::Text("Hello, world!");
        return true;
    }

   private:
    std::shared_ptr<USTC_CG::StrokeSystem> stroke_system;
};

int main()
{
    using namespace USTC_CG;

    std::shared_ptr<StrokeSystem> stroke_system;

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