

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ui_imgui.hpp"

#include <imgui_internal.h>

#include <string>

#include "imgui.h"
#include "imgui/blueprint-utilities/builders.h"
#include "imgui/blueprint-utilities/images.inl"
#include "imgui/blueprint-utilities/widgets.h"
#include "imgui/imgui-node-editor/imgui_node_editor.h"
#include "nodes/ui/imgui.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <fstream>

#include "RHI/rhi.hpp"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/core/socket.hpp"
#include "nodes/system/node_system.hpp"
#include "stb/stb_image.h"

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

static bool Splitter(
    bool split_vertically,
    float thickness,
    float* size1,
    float* size2,
    float min_size1,
    float min_size2,
    float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos +
             (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(
                          split_vertically
                              ? ImVec2(thickness, splitter_long_axis_size)
                              : ImVec2(splitter_long_axis_size, thickness),
                          0.0f,
                          0.0f);
    return SplitterBehavior(
        bb,
        id,
        split_vertically ? ImGuiAxis_X : ImGuiAxis_Y,
        size1,
        size2,
        min_size1,
        min_size2,
        0.0f);
}

NodeWidget::NodeWidget(const NodeWidgetSettings& desc)
    : storage_(desc.create_storage()),
      tree_(desc.system->get_node_tree()),
      system_(desc.system),
      widget_name(desc.WidgetName())
{
    ed::Config config;

    config.UserPointer = this;

    config.SaveSettings = [](const char* data,
                             size_t size,
                             ax::NodeEditor::SaveReasonFlags reason,
                             void* userPointer) -> bool {
        if (static_cast<bool>(
                reason & (NodeEditor::SaveReasonFlags::Navigation |
                          NodeEditor::SaveReasonFlags::Selection))) {
            return true;
        }
        auto ptr = static_cast<NodeWidget*>(userPointer);
        auto storage = ptr->storage_.get();

        std::string node_serialize = ptr->tree_->serialize();
        node_serialize.erase(node_serialize.end() - 1);

        auto ui_json = std::string(data + 1, size - 2);

        node_serialize += "," + ui_json + '}';

        storage->save(node_serialize);
        return true;
    };

    config.LoadSettings = [](void* userPointer) {
        auto ptr = static_cast<NodeWidget*>(userPointer);
        auto storage = ptr->storage_.get();

        std::string data = storage->load();
        if (!data.empty()) {
            ptr->tree_->Deserialize(data);
        }

        return data;
    };

    m_Editor = ed::CreateEditor(&config);

    m_HeaderBackground =
        LoadTexture(BlueprintBackground, sizeof(BlueprintBackground));
}

NodeWidget::~NodeWidget()
{
    ed::SetCurrentEditor(m_Editor);
    ed::DestroyEditor(m_Editor);
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
    if (first_draw) {
        first_draw = false;
        return true;
    }
    auto& io = ImGui::GetIO();

    ed::SetCurrentEditor(m_Editor);

    if (ed::GetSelectedObjectCount() > 0) {
        Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);
        ShowLeftPane(leftPaneWidth - 4.0f);
        ImGui::SameLine(0.0f, 12.0f);
    }

    ed::Begin(GetWindowUniqueName().c_str(), ImGui::GetContentRegionAvail());
    {
        auto cursorTopLeft = ImGui::GetCursorScreenPos();

        util::BlueprintNodeBuilder builder(
            m_HeaderBackground.Get(),
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
                    color = ImColor(18, 15, 16, 255);
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
            if (pin->node)
                ImGui::Text("Node: %p", pin->node->ID.AsPointer());
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

    if (tree_->GetDirty()) {
        system_->execute(true);
        tree_->SetDirty(false);
    }

    return true;
}

std::string NodeWidget::GetWindowUniqueName()
{
    if (!widget_name.empty()) {
        return widget_name;
    }
    return "NodeEditor##" +
           std::to_string(reinterpret_cast<uint64_t>(system_.get()));
}

const char* NodeWidget::GetWindowName()
{
    return "Node editor";
}

void NodeWidget::ShowLeftPane(float paneWidth)
{
    auto& io = ImGui::GetIO();

    std::vector<NodeId> selectedNodes;
    std::vector<LinkId> selectedLinks;
    selectedNodes.resize(ed::GetSelectedObjectCount());
    selectedLinks.resize(ed::GetSelectedObjectCount());

    int nodeCount = ed::GetSelectedNodes(
        selectedNodes.data(), static_cast<int>(selectedNodes.size()));
    int linkCount = ed::GetSelectedLinks(
        selectedLinks.data(), static_cast<int>(selectedLinks.size()));

    selectedNodes.resize(nodeCount);
    selectedLinks.resize(linkCount);
    ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

    ImGui::Text(
        "FPS: %.2f (%.2gms)",
        io.Framerate,
        io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    paneWidth = ImGui::GetContentRegionAvail().x;

    ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
    ImGui::Spring(0.0f, 0.0f);
    if (ImGui::Button("Zoom to Content"))
        ed::NavigateToContent();
    ImGui::Spring(0.0f);
    if (ImGui::Button("Show Flow")) {
        for (auto& link : tree_->links)
            ed::Flow(link->ID);
    }
    ImGui::Spring();
    ImGui::EndHorizontal();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImGui::GetCursorScreenPos(),
        ImGui::GetCursorScreenPos() +
            ImVec2(paneWidth, ImGui::GetTextLineHeight()),
        ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]),
        ImGui::GetTextLineHeight() * 0.25f);
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::TextUnformatted("Nodes");
    ImGui::Indent();
    for (auto& node : tree_->nodes) {
        ImGui::PushID(node->ID.AsPointer());
        auto start = ImGui::GetCursorScreenPos();

        if (const auto progress = GetTouchProgress(node->ID)) {
            ImGui::GetWindowDrawList()->AddLine(
                start + ImVec2(-8, 0),
                start + ImVec2(-8, ImGui::GetTextLineHeight()),
                IM_COL32(255, 0, 0, 255 - (int)(255 * progress)),
                4.0f);
        }

        bool isSelected =
            std::find(selectedNodes.begin(), selectedNodes.end(), node->ID) !=
            selectedNodes.end();
        ImGui::SetNextItemAllowOverlap();
        if (ImGui::Selectable(
                (node->ui_name + "##" +
                 std::to_string(
                     reinterpret_cast<uintptr_t>(node->ID.AsPointer())))
                    .c_str(),
                &isSelected)) {
            if (io.KeyCtrl) {
                if (isSelected)
                    ed::SelectNode(node->ID, true);
                else
                    ed::DeselectNode(node->ID);
            }
            else
                ed::SelectNode(node->ID, false);

            ed::NavigateToSelection();
        }
        ImGui::PopID();
    }
    ImGui::Unindent();

    static int changeCount = 0;

    ImGui::GetWindowDrawList()->AddRectFilled(
        ImGui::GetCursorScreenPos(),
        ImGui::GetCursorScreenPos() +
            ImVec2(paneWidth, ImGui::GetTextLineHeight()),
        ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]),
        ImGui::GetTextLineHeight() * 0.25f);
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::TextUnformatted("Selection");

    ImGui::Indent();
    for (int i = 0; i < nodeCount; ++i) {
        ImGui::Text("Node (%p)", selectedNodes[i].AsPointer());
        auto node = tree_->find_node(selectedNodes[i]);
        if (node->override_left_pane_info)
            node->override_left_pane_info();
    }

    for (int i = 0; i < linkCount; ++i)
        ImGui::Text("Link (%p)", selectedLinks[i].AsPointer());
    ImGui::Unindent();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
        for (auto& link : tree_->links)
            ed::Flow(link->ID);

    if (ed::HasSelectionChanged())
        ++changeCount;

    ImGui::GetWindowDrawList()->AddRectFilled(
        ImGui::GetCursorScreenPos(),
        ImGui::GetCursorScreenPos() +
            ImVec2(paneWidth, ImGui::GetTextLineHeight()),
        ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]),
        ImGui::GetTextLineHeight() * 0.25f);
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::TextUnformatted("Node Tree Info");

    ImGui::EndChild();
}

float NodeWidget::GetTouchProgress(NodeId id)
{
    auto it = m_NodeTouchTime.find(id);
    if (it != m_NodeTouchTime.end() && it->second > 0.0f)
        return (m_TouchTime - it->second) / m_TouchTime;
    else
        return 0.0f;
}

bool NodeWidget::draw_socket_controllers(NodeSocket* input)
{
    if (input->socket_group) {
        return false;
    }
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
        case entt::type_hash<bool>().value():
            changed |= ImGui::Checkbox(
                (input->ui_name + ("##" + std::to_string(input->ID.Get())))
                    .c_str(),
                &input->dataField.value.cast<bool&>());
            break;
    }

    return changed;
}

nvrhi::TextureHandle NodeWidget::LoadTexture(
    const unsigned char* data,
    size_t buffer_size)
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
        desc.initialState = nvrhi::ResourceStates::ShaderResource;
        desc.keepInitialState = true;

        auto [texture, _] = RHI::load_texture(desc, loaded_data);
        stbi_image_free(loaded_data);
        return texture;
    }
    else
        return nullptr;
}

ImGuiWindowFlags NodeWidget::GetWindowFlag()
{
    return ImGuiWindowFlags_NoScrollbar;
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

    const std::string typeName = get_type_name(type);
    auto hashValue_r = hashColorComponent("r", typeName);
    auto hashValue_g = hashColorComponent("g", typeName);
    auto hashValue_b = hashColorComponent("b", typeName);

    return ImColor(
        hashValue_r % 192 + 63, hashValue_g % 192 + 63, hashValue_b % 192 + 63);
}
NodeWidgetSettings::NodeWidgetSettings()
{
}

struct NodeSystemFileStorage : public NodeSystemStorage {
    explicit NodeSystemFileStorage(const std::filesystem::path& json_path)
        : json_path_(json_path)
    {
    }

    void save(const std::string& data) override
    {
        std::ofstream file(json_path_);
        file << data;
    }

    std::string load() override
    {
        std::ifstream file(json_path_);
        if (!file) {
            return std::string();
        }

        std::string data;
        file.seekg(0, std::ios_base::end);
        auto size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios_base::beg);

        data.reserve(size);
        data.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());

        return data;
    }

    std::filesystem::path json_path_;
};

std::unique_ptr<NodeSystemStorage> FileBasedNodeWidgetSettings::create_storage()
    const
{
    return std::make_unique<NodeSystemFileStorage>(json_path);
}

std::string FileBasedNodeWidgetSettings::WidgetName() const
{
    return json_path.string();
}

std::unique_ptr<IWidget> create_node_imgui_widget(
    const NodeWidgetSettings& desc)
{
    return std::make_unique<NodeWidget>(desc);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE