#pragma once
#include <functional>
#include <memory>

#include "GUI/widget.h"
#include "nodes/ui/api.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
class Stage;

namespace ax::NodeEditor {
enum class SaveReasonFlags : uint32_t;
}

class NodeSystem;

struct NODES_UI_IMGUI_API NodeSystemStorage {
    virtual ~NodeSystemStorage() = default;
    virtual void save(const std::string& data) = 0;
    virtual std::string load() = 0;
};

struct NODES_UI_IMGUI_API NodeWidgetSettings {
    virtual ~NodeWidgetSettings() = default;
    NodeWidgetSettings();

    virtual std::string WidgetName() const
    {
        return {};
    }

    std::shared_ptr<NodeSystem> system;
    virtual std::unique_ptr<NodeSystemStorage> create_storage() const = 0;

    friend class NodeWidget;
};

struct NODES_UI_IMGUI_API FileBasedNodeWidgetSettings
    : public NodeWidgetSettings {
    std::filesystem::path json_path;
    std::unique_ptr<NodeSystemStorage> create_storage() const override;

    std::string WidgetName() const override;
};

struct NullStorage : public NodeSystemStorage {
    void save(const std::string& data) override
    {
    }
    std::string load() override
    {
        return {};
    }
};

struct NODES_UI_IMGUI_API NullBasedNodeWidgetSettings
    : public NodeWidgetSettings {
    std::string widget_name;

    std::unique_ptr<NodeSystemStorage> create_storage() const override
    {
        return std::make_unique<NullStorage>();
    }

    std::string WidgetName() const override
    {
        return widget_name;
    }
};

NODES_UI_IMGUI_API std::unique_ptr<IWidget> create_node_imgui_widget(
    const NodeWidgetSettings& desc);

USTC_CG_NAMESPACE_CLOSE_SCOPE