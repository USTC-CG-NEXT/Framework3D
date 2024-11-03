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

struct NodeSystemStorage {
    virtual ~NodeSystemStorage() = default;
    virtual void save(const std::string& data) = 0;
    virtual std::string load() = 0;
};

struct NodeWidgetSettings {
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

struct FileBasedNodeWidgetSettings : public NodeWidgetSettings {
    std::filesystem::path json_path;
    std::unique_ptr<NodeSystemStorage> create_storage() const override;
};

std::unique_ptr<IWidget> create_node_imgui_widget(
    const NodeWidgetSettings& desc);

USTC_CG_NAMESPACE_CLOSE_SCOPE