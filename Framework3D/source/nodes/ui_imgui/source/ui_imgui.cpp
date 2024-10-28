#include "USTC_CG.h"
#include "nodes/ui/imgui.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>

#include "imgui.h"
#include "imgui/blueprint-utilities/builders.h"
#include "imgui/blueprint-utilities/images.inl"
#include "imgui/blueprint-utilities/widgets.h"
#include "imgui/imgui-node-editor/imgui_node_editor.h"

#define STB_IMAGE_IMPLEMENTATION

#include <fstream>

#include "RHI/rhi.hpp"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/core/socket.hpp"
#include "stb_image.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;

static ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}
class NodeWidget : public IWidget {
   public:
    explicit NodeWidget(NodeTree* tree);

    ~NodeWidget() override;
    Node* create_node_menu();
    bool BuildUI() override;

   private:
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
    static const int m_PinIconSize = 20;

    std::string widget_name;

    ed::EditorContext* m_Editor = nullptr;

    bool draw_socket_controllers(NodeSocket* input);

    nvrhi::TextureHandle LoadTexture(const unsigned char* data, size_t buffer_size)
    {
        int width = 0, height = 0, component = 0;
        if (auto loaded_data = stbi_load_from_memory(
                data, buffer_size, &width, &height, &component, 4)) {
            nvrhi::TextureDesc desc;
            desc.width = width;
            desc.height = height;
            desc.format = nvrhi::Format::RGBA8_UNORM;
            desc.isRenderTarget = false;
            desc.isUAV = false;
            desc.initialState = nvrhi::ResourceStates::Common;
            desc.keepInitialState = true;

            auto texture = rhi::load_texture(desc, data);
            stbi_image_free(loaded_data);
            return texture;
        }
        else
            return nullptr;
    }

    ImVector<nvrhi::TextureHandle> m_Textures;

    void DrawPinIcon(const NodeSocket& pin, bool connected, int alpha);

    static ImColor GetIconColor(SocketType type);
};

NodeWidget::NodeWidget(NodeTree* tree) : tree_(tree)
{
    ed::Config config;

    config.UserPointer = this;

    config.SaveSettings = [](const char* data,
                             size_t size,
                             NodeEditor::SaveReasonFlags reason,
                             void* userPointer) -> bool {
        auto ptr = static_cast<NodeWidget*>(userPointer);

        std::ofstream file("test.json");
        auto node_serialize = ptr->tree_->serialize();

        node_serialize.erase(node_serialize.end() - 1);

        auto ui_json = std::string(data + 1);
        ui_json.erase(ui_json.end() - 1);

        node_serialize += "," + ui_json + '}';

        file << node_serialize;
        return true;
    };

    config.LoadSettings = [](char* d, void* userPointer) -> size_t {
        auto ptr = static_cast<NodeWidget*>(userPointer);
        std::ifstream file("test.json");
        if (!file) {
            return 0;
        }
        if (!d) {
            file.seekg(0, std::ios_base::end);
            return file.tellg();
        }

        std::string data;
        file.seekg(0, std::ios_base::end);
        auto size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios_base::beg);

        data.reserve(size);
        data.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());

        if (data.size() > 0) {
            ptr->tree_->Deserialize(data);
        }

        memcpy(d, data.data(), data.size());

        return 0;
    };

    m_Editor = ed::CreateEditor(&config);

    m_HeaderBackground =
        LoadTexture(BlueprintBackground, sizeof(BlueprintBackground));
}

NodeWidget::~NodeWidget()
{
}

Node* NodeWidget::create_node_menu()
{
    auto& node_registry = tree_->get_descriptor().node_registry;

    Node* node = nullptr;

    for (auto&& value : node_registry) {
        auto name = value.second.ui_name;
        if (ImGui::MenuItem(name.c_str()))
            node = tree_->add_node(value.second.id_name.c_str());
    }

    return node;
}

bool NodeWidget::BuildUI()
{
    //// A very awkward workaround.
    // if (frame == 0) {
    //     frame++;
    //     return;
    // }
    // frame++;

    auto& io = ImGui::GetIO();

    ed::SetCurrentEditor(m_Editor);
    // Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);
    // ShowLeftPane(leftPaneWidth - 4.0f);
    ImGui::SameLine(0.0f, 12.0f);

    ed::Begin(("Node editor" + widget_name).c_str());
    {
        auto cursorTopLeft = ImGui::GetCursorScreenPos();

        util::BlueprintNodeBuilder builder(
            m_HeaderBackground,
            m_HeaderBackground->getDesc().width,
            m_HeaderBackground->getDesc().height);

        for (auto&& node : tree_->nodes) {
            if (node->typeinfo->INVISIBLE) {
                continue;
            }

            constexpr auto isSimple = false;

            builder.Begin(node->ID);
            if constexpr (!isSimple) {
                ImColor color;
                memcpy(&color, node->Color, sizeof(ImColor));
                if (node->MISSING_INPUT) {
                    color = ImColor(255, 206, 69, 255);
                }
                if (!node->REQUIRED) {
                    color = ImColor(84, 57, 56, 255);
                }

                if (!node->execution_failed.empty()) {
                    color = ImColor(255, 0, 0, 255);
                }
                builder.Header(color);
                ImGui::Spring(0);
                ImGui::TextUnformatted(node->ui_name.c_str());
                if (!node->execution_failed.empty()) {
                    ImGui::TextUnformatted(
                        (": " + node->execution_failed).c_str());
                }
                ImGui::Spring(1);
                ImGui::Dummy(ImVec2(0, 28));
                ImGui::Spring(0);
                builder.EndHeader();
            }

            for (auto& input : node->get_inputs()) {
                auto alpha = ImGui::GetStyle().Alpha;
                if (newLinkPin && !tree_->can_create_link(newLinkPin, input) &&
                    input != newLinkPin)
                    alpha = alpha * (48.0f / 255.0f);

                builder.Input(input->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                DrawPinIcon(
                    *input,
                    tree_->is_pin_linked(input->ID),
                    (int)(alpha * 255));
                ImGui::Spring(0);

                if (tree_->is_pin_linked(input->ID)) {
                    ImGui::TextUnformatted(input->ui_name);
                    ImGui::Spring(0);
                }
                else {
                    ImGui::PushItemWidth(120.0f);
                    if (draw_socket_controllers(input))
                        tree_->SetDirty();
                    ImGui::PopItemWidth();
                    ImGui::Spring(0);
                }

                ImGui::PopStyleVar();
                builder.EndInput();
            }

            if (isSimple) {
                builder.Middle();

                ImGui::Spring(1, 0);
                ImGui::TextUnformatted(node->ui_name.c_str());
                ImGui::Spring(1, 0);
            }

            for (auto& output : node->get_outputs()) {
                auto alpha = ImGui::GetStyle().Alpha;
                if (newLinkPin && !tree_->can_create_link(newLinkPin, output) &&
                    output != newLinkPin)
                    alpha = alpha * (48.0f / 255.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                builder.Output(output->ID);

                ImGui::Spring(0);
                ImGui::TextUnformatted(output->ui_name);
                ImGui::Spring(0);
                DrawPinIcon(
                    *output,
                    tree_->is_pin_linked(output->ID),
                    (int)(alpha * 255));
                ImGui::PopStyleVar();

                builder.EndOutput();
            }

            builder.End();
        }

        for (auto&& node : tree_->nodes) {
            const float commentAlpha = 0.75f;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
            ed::PushStyleColor(
                ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
            ed::PushStyleColor(
                ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
            ed::BeginNode(node->ID);
            ImGui::PushID(node->ID.AsPointer());
            ImGui::BeginVertical("content");
            ImGui::BeginHorizontal("horizontal");
            ImGui::Spring(1);
            ImGui::TextUnformatted(node->ui_name.c_str());
            ImGui::Spring(1);
            ImGui::EndHorizontal();
            ImVec2 size;
            memcpy(&size, node->Size, sizeof(ImVec2));
            ed::Group(size);
            ImGui::EndVertical();
            ImGui::PopID();
            ed::EndNode();
            ed::PopStyleColor(2);
            ImGui::PopStyleVar();

            if (ed::BeginGroupHint(node->ID)) {
                auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

                auto min = ed::GetGroupMin();
                ImGui::SetCursorScreenPos(
                    min -
                    ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                ImGui::BeginGroup();
                ImGui::TextUnformatted(node->ui_name.c_str());
                ImGui::EndGroup();

                auto drawList = ed::GetHintBackgroundDrawList();

                auto hintBounds = ImGui_GetItemRect();
                auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

                drawList->AddRectFilled(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 64 * bgAlpha / 255),
                    4.0f);

                drawList->AddRect(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 128 * bgAlpha / 255),
                    4.0f);
            }
            ed::EndGroupHint();
        }

        for (std::unique_ptr<NodeLink>& link : tree_->links) {
            auto type = link->from_sock->type_info;
            if (!type)
                type = link->to_sock->type_info;

            ImColor color = GetIconColor(type);

            auto linkId = link->ID;
            auto startPin = link->StartPinID;
            auto endPin = link->EndPinID;

            // If there is an invisible node after the link, use the first as
            // the id for the ui link
            if (link->nextLink) {
                endPin = link->nextLink->to_sock->ID;
            }

            ed::Link(linkId, startPin, endPin, color, 2.0f);
        }

        if (!createNewNode) {
            if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f)) {
                auto showLabel = [](const char* label, ImColor color) {
                    ImGui::SetCursorPosY(
                        ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                    auto size = ImGui::CalcTextSize(label);

                    auto padding = ImGui::GetStyle().FramePadding;
                    auto spacing = ImGui::GetStyle().ItemSpacing;

                    ImGui::SetCursorPos(
                        ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                    auto rectMin = ImGui::GetCursorScreenPos() - padding;
                    auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                    auto drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(
                        rectMin, rectMax, color, size.y * 0.15f);
                    ImGui::TextUnformatted(label);
                };

                SocketID startPinId = 0, endPinId = 0;
                if (ed::QueryNewLink(&startPinId, &endPinId)) {
                    auto startPin = tree_->find_pin(startPinId);
                    auto endPin = tree_->find_pin(endPinId);

                    newLinkPin = startPin ? startPin : endPin;

                    if (startPin && endPin) {
                        if (tree_->can_create_link(startPin, endPin)) {
                            showLabel(
                                "+ Create Link", ImColor(32, 45, 32, 180));
                            if (ed::AcceptNewItem(
                                    ImColor(128, 255, 128), 4.0f)) {
                                tree_->add_link(startPinId, endPinId);
                            }
                        }
                    }
                }

                SocketID pinId = 0;
                if (ed::QueryNewNode(&pinId)) {
                    newLinkPin = tree_->find_pin(pinId);
                    if (newLinkPin)
                        showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                    if (ed::AcceptNewItem()) {
                        createNewNode = true;
                        newNodeLinkPin = tree_->find_pin(pinId);
                        newLinkPin = nullptr;
                        ed::Suspend();
                        ImGui::OpenPopup("Create New Node");
                        ed::Resume();
                    }
                }
            }
            else
                newLinkPin = nullptr;

            ed::EndCreate();

            if (ed::BeginDelete()) {
                NodeId nodeId = 0;
                while (ed::QueryDeletedNode(&nodeId)) {
                    if (ed::AcceptDeletedItem()) {
                        auto id = std::find_if(
                            tree_->nodes.begin(),
                            tree_->nodes.end(),
                            [nodeId](auto& node) {
                                return node->ID == nodeId;
                            });
                        if (id != tree_->nodes.end())
                            tree_->delete_node(nodeId);
                    }
                }

                LinkId linkId = 0;
                while (ed::QueryDeletedLink(&linkId)) {
                    if (ed::AcceptDeletedItem()) {
                        tree_->delete_link(linkId);
                    }
                }

                // tree_->trigger_refresh_topology();
            }
            ed::EndDelete();
        }

        ImGui::SetCursorScreenPos(cursorTopLeft);
    }

    auto openPopupPosition = ImGui::GetMousePos();
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&contextNodeId))
        ImGui::OpenPopup("Node Context Menu");
    else if (ed::ShowPinContextMenu(&contextPinId))
        ImGui::OpenPopup("Pin Context Menu");
    else if (ed::ShowLinkContextMenu(&contextLinkId))
        ImGui::OpenPopup("Link Context Menu");
    else if (ed::ShowBackgroundContextMenu()) {
        ImGui::OpenPopup("Create New Node");
        newNodeLinkPin = nullptr;
    }
    ed::Resume();

    ed::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("Node Context Menu")) {
        auto node = tree_->find_node(contextNodeId);

        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();
        if (node) {
            ImGui::Text("ID: %p", node->ID.AsPointer());
            ImGui::Text("Inputs: %d", (int)node->get_inputs().size());
            ImGui::Text("Outputs: %d", (int)node->get_outputs().size());
        }
        else
            ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteNode(contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu")) {
        auto pin = tree_->find_pin(contextPinId);

        ImGui::TextUnformatted("Pin Context Menu");
        ImGui::Separator();
        if (pin) {
            ImGui::Text("ID: %p", pin->ID.AsPointer());
            if (pin->Node)
                ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
            else
                ImGui::Text("Node: %s", "<none>");
        }
        else
            ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu")) {
        auto link = tree_->find_link(contextLinkId);

        ImGui::TextUnformatted("Link Context Menu");
        ImGui::Separator();
        if (link) {
            ImGui::Text("ID: %p", link->ID.AsPointer());
            ImGui::Text("From: %p", link->StartPinID.AsPointer());
            ImGui::Text("To: %p", link->EndPinID.AsPointer());
        }
        else
            ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteLink(contextLinkId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Create New Node")) {
        if (!location_remembered) {
            newNodePostion = openPopupPosition;
            location_remembered = true;
        }

        Node* node = create_node_menu();
        // ImGui::Separator();
        // if (ImGui::MenuItem("Comment"))
        //     node = SpawnComment();

        if (node) {
            location_remembered = false;
            createNewNode = false;
            tree_->SetDirty();

            ed::SetNodePosition(node->ID, newNodePostion);

            if (auto startPin = newNodeLinkPin) {
                auto& pins = startPin->in_out == PinKind::Input
                                 ? node->get_outputs()
                                 : node->get_inputs();

                for (auto& pin : pins) {
                    if (tree_->can_create_link(startPin, pin)) {
                        auto endPin = pin;

                        tree_->add_link(startPin->ID, endPin->ID);

                        break;
                    }
                }
            }
        }

        ImGui::EndPopup();
    }
    else
        createNewNode = false;
    ImGui::PopStyleVar();
    ed::Resume();

    ed::End();

    // auto editorMin = ImGui::GetItemRectMin();
    // auto editorMax = ImGui::GetItemRectMax();

    // if (node_system_type != NodeSystemType::Render) {
    //     tree_->try_execution();
    // }
    return true;
}

bool NodeWidget::draw_socket_controllers(NodeSocket* input)
{
    bool changed = false;
    switch (input->type_info.id()) {
        default:
            ImGui::TextUnformatted(input->ui_name);
            ImGui::Spring(0);
            break;
        case entt::type_hash<int>().value():
            changed |= ImGui::SliderInt(
                (input->ui_name + ("##" + std::to_string(input->ID.Get())))
                    .c_str(),
                &input->dataField.value.cast<int&>(),
                input->dataField.min.cast<int>(),
                input->dataField.max.cast<int>());
            break;

        case entt::type_hash<float>().value():
            changed |= ImGui::SliderFloat(
                (input->ui_name + ("##" + std::to_string(input->ID.Get())))
                    .c_str(),
                &input->dataField.value.cast<float&>(),
                input->dataField.min.cast<float>(),
                input->dataField.max.cast<float>());
            break;

        case entt::type_hash<std::string>().value():
            changed |= ImGui::InputText(
                (input->ui_name + ("##" + std::to_string(input->ID.Get())))
                    .c_str(),
                input->dataField.value.cast<std::string&>().data(),
                255);
            break;
    }

    return changed;
}

void NodeWidget::DrawPinIcon(const NodeSocket& pin, bool connected, int alpha)
{
    IconType iconType;

    ImColor color = GetIconColor(pin.type_info);

    if (!pin.type_info) {
        if (pin.directly_linked_sockets.size() > 0) {
            color = GetIconColor(pin.directly_linked_sockets[0]->type_info);
        }
    }

    color.Value.w = alpha / 255.0f;
    iconType = IconType::Circle;

    Widgets::Icon(
        ImVec2(
            static_cast<float>(m_PinIconSize),
            static_cast<float>(m_PinIconSize)),
        iconType,
        connected,
        color,
        ImColor(32, 32, 32, alpha));
}

ImColor NodeWidget::GetIconColor(SocketType type)
{
    auto hashColorComponent = [](const std::string& prefix,
                                 const std::string& typeName) {
        return static_cast<int>(
            entt::hashed_string{ (prefix + typeName).c_str() }.value());
    };

    const std::string typeName = std::string(type.info().name());
    auto hashValue_r = hashColorComponent("r", typeName);
    auto hashValue_g = hashColorComponent("g", typeName);
    auto hashValue_b = hashColorComponent("b", typeName);

    return ImColor(hashValue_r % 255, hashValue_g % 255, hashValue_b % 255);
}

std::unique_ptr<IWidget> create_node_imgui_widget(NodeTree* tree)
{
    return std::make_unique<NodeWidget>(tree);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE