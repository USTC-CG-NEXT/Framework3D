#pragma once

#include <pxr/usd/sdf/path.h>

#include <fstream>
#include <string>

#include "nodes/ui/imgui.hpp"
#include "stage/stage.hpp"

struct UsdBasedNodeStorage : public USTC_CG::NodeSystemStorage {
    UsdBasedNodeStorage(USTC_CG::Stage* stage, const pxr::SdfPath& path)
        : stage_(stage),
          path_(path)
    {
    }
    void save(const std::string& data) override;
    std::string load() override;
    pxr::SdfPath path_;
    USTC_CG::Stage* stage_;
};

struct UsdBasedNodeWidgetSettings : public USTC_CG::NodeWidgetSettings {
    pxr::SdfPath json_path;
    USTC_CG::Stage* stage;

    std::string WidgetName() const override;

    std::unique_ptr<USTC_CG::NodeSystemStorage> create_storage() const override;
};

inline void UsdBasedNodeStorage::save(const std::string& data)
{
    stage_->save_string_to_usd(path_, data);
}

inline std::string UsdBasedNodeStorage::load()
{
    return stage_->load_string_from_usd(path_);
}

inline std::string UsdBasedNodeWidgetSettings::WidgetName() const
{
    return json_path.GetString();
}

inline std::unique_ptr<USTC_CG::NodeSystemStorage>
UsdBasedNodeWidgetSettings::create_storage() const
{
    return std::make_unique<UsdBasedNodeStorage>(stage, json_path);
}