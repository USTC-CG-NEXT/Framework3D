#pragma once
#include <filesystem>
#include <memory>

#include "GUI/widget.h"
#include "pxr/usd/usd/stage.h"
#include "widgets/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Stage;

class UsdFileViewer : public IWidget {
   public:
    explicit UsdFileViewer(Stage* stage);

    ~UsdFileViewer() override;

    bool BuildUI() override;

   protected:
    bool Begin() override
    {
        return true;
    }

    void End() override
    {
    }


   private:
    void ShowFileTree();
    void ShowPrimInfo();
    void select_file();

    void show_right_click_menu();
    void DrawChild(const pxr::UsdPrim& prim);

    pxr::SdfPath selected;
    Stage* stage;
    bool is_selecting_file = false;
    pxr::SdfPath selecting_file_base;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE