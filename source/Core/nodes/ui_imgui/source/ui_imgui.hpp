#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include <string>

#include "RHI/rhi.hpp"
#include "imgui.h"
#include "imgui/blueprint-utilities/builders.h"
#include "imgui/blueprint-utilities/images.inl"
#include "imgui/blueprint-utilities/widgets.h"
#include "imgui/imgui-node-editor/imgui_node_editor.h"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/core/socket.hpp"
#include "nodes/system/node_system.hpp"
#include "nodes/ui/imgui.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;

struct NodeIdLess {
    bool operator()(const NodeId& lhs, const NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};

class NodeWidget : public IWidget {
   public:
    explicit NodeWidget(const NodeWidgetSettings& desc);

    ~NodeWidget() override;
    std::vector<Node*> create_node_menu(bool cursor);
    bool BuildUI() override;

   protected:
    std::string GetWindowUniqueName() override;

    const char* GetWindowName() override;

    void SetNodeSystemDirty(bool dirty) override;

   private:
    void ShowLeftPane(float paneWidth);

    float GetTouchProgress(NodeId id);
    const float m_TouchTime = 1.0f;
    std::map<NodeId, float, NodeIdLess> m_NodeTouchTime;

    std::unique_ptr<NodeSystemStorage> storage_;

    NodeTree* tree_;
    bool createNewNode = false;
    NodeSocket* newNodeLinkPin = nullptr;
    NodeSocket* newLinkPin = nullptr;

    NodeId contextNodeId = 0;
    LinkId contextLinkId = 0;
    SocketID contextPinId = 0;

    nvrhi::TextureHandle m_HeaderBackground = nullptr;
    ImVec2 newNodePostion;
    bool location_remembered = false;
    std::shared_ptr<NodeSystem> system_;
    bool create_new_node_search_cursor;
    static const int m_PinIconSize = 20;

    std::string widget_name;

    ed::EditorContext* m_Editor = nullptr;

    float leftPaneWidth = 400.0f;
    float rightPaneWidth = 800.0f;

    bool first_draw = true;

    bool draw_socket_controllers(NodeSocket* input);

    static nvrhi::TextureHandle LoadTexture(
        const unsigned char* data,
        size_t buffer_size);

    ImGuiWindowFlags GetWindowFlag() override;

    ImVector<nvrhi::TextureHandle> m_Textures;

    void DrawPinIcon(const NodeSocket& pin, bool connected, int alpha);

    static ImColor GetIconColor(SocketType type);

    void ShowInputOrOutput(
        const NodeSocket& socket,
        const entt::meta_any& value);

    std::vector<Node*> add_node(const std::string& id_name);
};

USTC_CG_NAMESPACE_CLOSE_SCOPE