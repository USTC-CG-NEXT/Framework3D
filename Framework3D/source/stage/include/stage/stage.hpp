#pragma once
#include <pxr/usd/usd/stage.h>

#include "USTC_CG.h"
USTC_CG_NAMESPACE_OPEN_SCOPE

class USTC_CG_API Stage {
   public:
    Stage();

    pxr::UsdPrim add_prim(const pxr::SdfPath& path);

    std::string stage_content() const;

   private:
    pxr::UsdStageRefPtr stage;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE