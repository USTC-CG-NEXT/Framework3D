#pragma once
#include <memory>

#include "GUI/widget.h"
#include "USTC_CG.h"
#include "pxr/usd/usd/stage.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class UsdFileViewer : public IWidget {
   public:
    explicit UsdFileViewer(const pxr::UsdStageRefPtr& stage);

    ~UsdFileViewer() override;

    bool BuildUI() override;

   private:
    void ShowFileTree();
    void ShowPrimInfo();
    pxr::SdfPath emit_editor_info_path();

    void show_right_click_menu();
    void DrawChild(const pxr::UsdPrim& prim);

    pxr::SdfPath selected;
    pxr::SdfPath editor_info_path;
    pxr::UsdStageRefPtr stage;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE