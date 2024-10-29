#include "stage/stage.hpp"

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
USTC_CG_NAMESPACE_OPEN_SCOPE

Stage::Stage()
{
    stage = pxr::UsdStage::CreateNew("stage.usda");
}

pxr::UsdPrim Stage::add_prim(const pxr::SdfPath& path)
{
    return stage->DefinePrim(path);
}

std::string Stage::stage_content() const
{
    std::string str;
    stage->GetRootLayer()->ExportToString(&str);
    return str;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
