#pragma once
#include <pxr/usd/usd/stage.h>

#include "stage/api.h"
USTC_CG_NAMESPACE_OPEN_SCOPE

class STAGE_API Stage {
   public:
    Stage();

    pxr::UsdPrim add_prim(const pxr::SdfPath& path);

    std::string stage_content() const;

   private:
    pxr::UsdStageRefPtr stage;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE