//  A general file containing functions related to nodes.

#include "nodes/core/node.hpp"

#include "entt/meta/resolve.hpp"
#include "nodes/core/api.h"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

void NodeLink::Serialize(nlohmann::json& value)
{
    if (!fromLink) {
        auto& link = value[std::to_string(ID.Get())];
        link["ID"] = ID.Get();

        auto startPin = StartPinID.Get();
        auto endPin = EndPinID.Get();

        link["StartPinID"] = startPin;
        if (nextLink) {
            endPin = nextLink->EndPinID.Get();
        }
        link["EndPinID"] = endPin;
    }
}

NodeTypeInfo::NodeTypeInfo(const char* id_name)
    : id_name(id_name),
      ui_name(id_name)
{
}

NodeTypeInfo& NodeTypeInfo::set_ui_name(const std::string& ui_name)
{
    this->ui_name = ui_name;
    return *this;
}

NodeTypeInfo& NodeTypeInfo::set_declare_function(
    const NodeDeclareFunction& decl_function)
{
    this->declare = decl_function;
    build_node_declaration();
    return *this;
}

NodeTypeInfo& NodeTypeInfo::set_execution_function(
    const ExecFunction& exec_function)
{
    this->node_execute = exec_function;
    return *this;
}

NodeTypeInfo& NodeTypeInfo::set_always_required(bool always_required)
{
    this->ALWAYS_REQUIRED = always_required;
    return *this;
}

void NodeTypeInfo::reset_declaration()
{
    static_declaration = NodeDeclaration();
}

void NodeTypeInfo::build_node_declaration()
{
    reset_declaration();
    NodeDeclarationBuilder node_decl_builder{ static_declaration };
    declare(node_decl_builder);
}

Node::Node(NodeTree* node_tree, int id, const char* idname)
    : ID(id),
      ui_name("Unknown"),
      tree_(node_tree)
{
    valid_ = pre_init_node(idname);
}

SocketGroup* Node::find_socket_group(
    const std::string& group_name,
    PinKind inout)
{
    auto group = std::find_if(
        socket_groups.begin(),
        socket_groups.end(),
        [&group_name, inout](const auto& group) {
            return group->identifier == group_name && group->kind == inout;
        });

    if (group == socket_groups.end()) {
        return nullptr;
    }

    return group->get();
}

Node::Node(NodeTree* node_tree, const char* idname)
    : ui_name("Unknown"),
      tree_(node_tree)
{
    ID = tree_->UniqueID();
    valid_ = pre_init_node(idname);
}

Node::~Node()
{
}

bool Node::is_node_group()
{
    return false;
}

void Node::serialize(nlohmann::json& value)
{
    if (!typeinfo->INVISIBLE) {
        auto& node = value[std::to_string(ID.Get())];
        node["ID"] = ID.Get();
        node["id_name"] = typeinfo->id_name;
        auto& input_socket_json = node["inputs"];
        auto& output_socket_json = node["outputs"];

        for (int i = 0; i < inputs.size(); ++i) {
            input_socket_json[std::to_string(i)] = inputs[i]->ID.Get();
        }

        for (int i = 0; i < outputs.size(); ++i) {
            output_socket_json[std::to_string(i)] = outputs[i]->ID.Get();
        }

        if (paired_node) {
            node["paired_node"] = paired_node->ID.Get();
        }

        for (int i = 0; i < socket_groups.size(); ++i) {
            socket_groups[i]->serialize(node);
        }
    }
}

void Node::register_socket_to_node(NodeSocket* socket, PinKind in_out)
{
    if (in_out == PinKind::Input) {
        inputs.push_back(socket);

        if (!socket->socket_group_identifier.empty()) {
            auto group = std::find_if(
                socket_groups.begin(),
                socket_groups.end(),
                [&socket](const auto& group) {
                    return group->identifier == socket->socket_group_identifier;
                });

            if (group == socket_groups.end()) {
                throw std::runtime_error("Socket group not found.");
            }

            auto location = (*group)->runtime_dynamic
                                ? (*group)->sockets.end() - 1
                                : (*group)->sockets.end();
            socket->socket_group = (*group).get();
            (*group)->sockets.insert(location, socket);
        }
    }
    else {
        if (!socket->socket_group_identifier.empty()) {
            auto group = std::find_if(
                socket_groups.begin(),
                socket_groups.end(),
                [&socket](const auto& group) {
                    return group->identifier == socket->socket_group_identifier;
                });
            if (group == socket_groups.end()) {
                throw std::runtime_error("Socket group not found.");
            }

            auto location = (*group)->runtime_dynamic
                                ? (*group)->sockets.end() - 1
                                : (*group)->sockets.end();
            socket->socket_group = (*group).get();
            (*group)->sockets.insert(location, socket);
        }

        outputs.push_back(socket);
    }
}

NodeSocket* Node::get_output_socket(const char* identifier) const
{
    return find_socket(identifier, PinKind::Output);
}

NodeSocket* Node::get_input_socket(const char* identifier) const
{
    return find_socket(identifier, PinKind::Input);
}

NodeSocket* Node::find_socket(const char* identifier, PinKind in_out) const
{
    const std::vector<NodeSocket*>* socket_group;

    if (in_out == PinKind::Input) {
        socket_group = &inputs;
    }
    else {
        socket_group = &outputs;
    }

    const auto id = find_socket_id(identifier, in_out);
    return (*socket_group)[id];
}

size_t Node::find_socket_id(const char* identifier, PinKind in_out) const
{
    int counter = 0;

    const std::vector<NodeSocket*>* socket_group;

    if (in_out == PinKind::Input) {
        socket_group = &inputs;
    }
    else {
        socket_group = &outputs;
    }

    for (NodeSocket* socket : *socket_group) {
        if (std::string(socket->identifier) == identifier) {
            return counter;
        }
        counter++;
    }
    assert(false);
    return -1;
}

std::vector<size_t> Node::find_socket_group_ids(
    const std::string& group_identifier,
    PinKind in_out) const
{
    std::vector<size_t> ids;
    const std::vector<NodeSocket*>* socket_group;

    if (in_out == PinKind::Input) {
        socket_group = &inputs;
    }
    else {
        socket_group = &outputs;
    }

    for (size_t i = 0; i < socket_group->size(); ++i) {
        if ((*socket_group)[i]->socket_group_identifier == group_identifier &&
            !(*socket_group)[i]->is_placeholder()) {
            ids.push_back(i);
        }
    }

    return ids;
}

const std::vector<NodeSocket*>& Node::get_inputs() const
{
    return inputs;
}

const std::vector<NodeSocket*>& Node::get_outputs() const
{
    return outputs;
}

bool Node::valid()
{
    return valid_;
}

void Node::generate_sockets_based_on_declaration(
    const SocketDeclaration& socket_declaration,
    const std::vector<NodeSocket*>& old_sockets,
    std::vector<NodeSocket*>& new_sockets)
{
    // TODO: This is a badly implemented zone. Refactor this.
    NodeSocket* new_socket;
    auto old_socket = std::find_if(
        old_sockets.begin(),
        old_sockets.end(),
        [&socket_declaration, this](NodeSocket* socket) {
            bool still_contained_in_current_declaration =
                std::string(socket->identifier) ==
                    socket_declaration.identifier &&
                socket->in_out == socket_declaration.in_out &&
                socket->type_info == socket_declaration.type;

            return still_contained_in_current_declaration;
        });
    if (old_socket != old_sockets.end()) {
        (*old_socket)->node = this;
        new_socket = *old_socket;
        new_socket->type_info = socket_declaration.type;
        socket_declaration.update_default_value(new_socket);
    }
    else {
        new_socket = socket_declaration.build(tree_, this);
    }
    new_sockets.push_back(new_socket);
}

void Node::generate_socket_groups_socket(
    const SocketGroup* socket_group,
    const std::vector<NodeSocket*>& old_sockets,
    std::vector<NodeSocket*>& new_sockets)
{
    for (auto&& socket_in_group : socket_group->sockets) {
        auto old_socket = std::find_if(
            old_sockets.begin(),
            old_sockets.end(),
            [&socket_in_group](NodeSocket* socket) {
                return std::string(socket->identifier) ==
                           socket_in_group->identifier &&
                       socket->in_out == socket_in_group->in_out;
            });
        if (old_socket != old_sockets.end()) {
            (*old_socket)->node = this;
            new_sockets.push_back(*old_socket);
        }
        else {
            log::info("Creating new socket in the group.");
        }
    }
}

NodeSocket* Node::add_socket(
    const char* type_name,
    const char* identifier,
    const char* name,
    PinKind in_out)
{
    auto socket = new NodeSocket(tree_->UniqueID());

    socket->type_info = get_socket_type(type_name);
    strcpy(socket->identifier, identifier);
    strcpy(socket->ui_name, name);
    socket->in_out = in_out;
    socket->node = this;

    register_socket_to_node(socket, in_out);

    tree_->sockets.emplace_back(socket);
    return socket;
}

NodeSocket* Node::group_add_socket(
    const std::string& socket_group_identifier,
    const char* type_name,
    const char* identifier,
    const char* name,
    PinKind in_out)
{
    auto group = std::find_if(
        socket_groups.begin(),
        socket_groups.end(),
        [&socket_group_identifier, in_out](const auto& group) {
            return group->identifier == socket_group_identifier &&
                   group->kind == in_out;
        });

    if (group == socket_groups.end()) {
        throw std::runtime_error("Socket group not found.");
    }

    auto socket = (*group)->add_socket(type_name, identifier, name);

    refresh_node();

    return socket;
}

void Node::group_remove_socket(
    const std::string& group_identifier,
    const char* identifier,
    PinKind in_out,
    bool is_recursive_call)
{
    auto group = std::find_if(
        socket_groups.begin(),
        socket_groups.end(),
        [&group_identifier, in_out](const auto& group) {
            return group->identifier == group_identifier &&
                   group->kind == in_out;
        });

    if (group == socket_groups.end()) {
        throw std::runtime_error("Socket group not found.");
    }

    if (!is_recursive_call &&
            (*group)->node->typeinfo->id_name == NODE_GROUP_IN_IDENTIFIER ||
        (*group)->node->typeinfo->id_name == NODE_GROUP_OUT_IDENTIFIER) {
        auto parent_node = (*group)->node->tree_->parent_node;

        if (parent_node) {
            if (in_out == PinKind::Input) {
                assert(group_identifier == InsideInputsPH);

                parent_node->group_remove_socket(
                    OutsideOutputsPH, identifier, in_out, true);
            }
            else {
                assert(group_identifier == InsideOutputsPH);
                parent_node->group_remove_socket(
                    OutsideInputsPH, identifier, in_out, true);
            }
        }
    }
    else {
        (*group)->remove_socket(identifier);
    }
}

void Node::remove_outdated_socket(NodeSocket* socket, PinKind kind)
{
    switch (kind) {
        case PinKind::Output:
            if (std::find(outputs.begin(), outputs.end(), socket) ==
                outputs.end()) {
                // If the sockets is not in the refreshed sockets
                auto out_dated_socket = std::find_if(
                    tree_->sockets.begin(),
                    tree_->sockets.end(),
                    [socket](auto&& ptr) { return socket == ptr.get(); });
                tree_->sockets.erase(out_dated_socket);
            }
            break;
        case PinKind::Input:
            if (std::find(inputs.begin(), inputs.end(), socket) ==
                inputs.end()) {
                // If the sockets is not in the refreshed sockets
                auto out_dated_socket = std::find_if(
                    tree_->sockets.begin(),
                    tree_->sockets.end(),
                    [socket](auto&& ptr) { return socket == ptr.get(); });
                tree_->sockets.erase(out_dated_socket);
            }
            break;
        default: break;
    }
}

void Node::out_date_sockets(
    const std::vector<NodeSocket*>& olds,
    PinKind pin_kind)
{
    for (auto old : olds) {
        remove_outdated_socket(old, pin_kind);
    }
}

// This function really synchronize the node to the node tree. After doing local
// operation like add socket, remove socket, deserialization, call this.

void Node::refresh_node()
{
    auto ntype = typeinfo;

    auto& node_decl = ntype->static_declaration;

    auto old_inputs = get_inputs();
    auto old_outputs = get_outputs();
    std::vector<NodeSocket*> new_inputs;
    std::vector<NodeSocket*> new_outputs;

    for (const SocketDeclaration* socket_decl : node_decl.inputs) {
        generate_sockets_based_on_declaration(
            *socket_decl, old_inputs, new_inputs);
    }

    for (const SocketDeclaration* socket_decl : node_decl.outputs) {
        generate_sockets_based_on_declaration(
            *socket_decl, old_outputs, new_outputs);
    }

    for (auto&& group : socket_groups) {
        generate_socket_groups_socket(group.get(), old_inputs, new_inputs);
        generate_socket_groups_socket(group.get(), old_outputs, new_outputs);
    }

    inputs = new_inputs;
    outputs = new_outputs;

    out_date_sockets(old_inputs, PinKind::Input);
    out_date_sockets(old_outputs, PinKind::Output);
}

void Node::deserialize(const nlohmann::json& node_json)
{
    for (auto&& input_id : node_json["inputs"]) {
        assert(tree_->find_pin(input_id.get<unsigned>()));
        register_socket_to_node(
            tree_->find_pin(input_id.get<unsigned>()), PinKind::Input);
    }

    for (auto&& output_id : node_json["outputs"]) {
        assert(tree_->find_pin(output_id.get<unsigned>()));
        register_socket_to_node(
            tree_->find_pin(output_id.get<unsigned>()), PinKind::Output);
    }

    for (auto&& group : socket_groups) {
        group->deserialize(node_json);
    }

    if (node_json.contains("paired_node")) {
        auto find_node =
            tree_->find_node(node_json["paired_node"].get<unsigned>());
        if (find_node) {
            paired_node = find_node;
            find_node->paired_node = this;
        }
    }

    refresh_node();
}

bool Node::pre_init_node(const char* idname)
{
    typeinfo = nodeTypeFind(idname);
    if (!typeinfo) {
        assert(false);
        return false;
    }
    ui_name = typeinfo->ui_name;
    memcpy(Color, typeinfo->color, sizeof(float) * 4);

    auto& node_decl = typeinfo->static_declaration;

    for (auto socket_group_declaration : node_decl.socket_group_decls) {
        socket_groups.emplace_back(
            socket_group_declaration->build(tree_, this));
    }

    return true;
}

const NodeTypeInfo* Node::nodeTypeFind(const char* idname)
{
    if (idname[0]) {
        const NodeTypeInfo* nt = tree_->descriptor_->get_node_type(idname);

        if (nt)
            return nt;
    }
    throw std::runtime_error("Id name not found.");
}

NodeGroup::NodeGroup(NodeTree* node_tree, const char* idname)
    : Node(node_tree, idname)
{
    ui_name = "Group";
    sub_tree = std::make_shared<NodeTree>(tree_->get_descriptor());
}

NodeGroup::NodeGroup(NodeTree* node_tree, int id, const char* idname)
    : Node(node_tree, id, idname)
{
    ui_name = "Group";
    sub_tree = std::make_shared<NodeTree>(tree_->get_descriptor());
}

bool NodeGroup::is_node_group()
{
    return true;
}

void NodeGroup::serialize(nlohmann::json& value)
{
    Node::serialize(value);
    std::string sub_tree_key =
        "sub_tree_" +
        std::to_string(reinterpret_cast<uintptr_t>(sub_tree.get())) + "_ptr";

    auto& node = value[std::to_string(ID.Get())];
    node["subtree"] = sub_tree_key;

    auto& sub_tree_json_location = value["sub_trees"][sub_tree_key];

    if (!value.contains(sub_tree_json_location)) {
        sub_tree_json_location = sub_tree->serialize();
    }
}

NodeSocket* NodeGroup::group_add_socket(
    const std::string& socket_group_identifier,
    const char* type_name,
    const char* identifier,
    const char* name,
    PinKind in_out)
{
    assert(
        socket_group_identifier == OutsideInputsPH ||
        socket_group_identifier == OutsideOutputsPH);
    if (in_out == PinKind::Input) {
        return node_group_add_input_socket(type_name, identifier, name).first;
    }
    else {
        return node_group_add_output_socket(type_name, identifier, name).first;
    }
}

void NodeGroup::group_remove_socket(
    const std::string& group_identifier,
    const char* identifier,
    PinKind in_out,
    bool is_recursive_call)
{
    assert(
        group_identifier == OutsideInputsPH ||
        group_identifier == OutsideOutputsPH);

    auto socket = find_socket(identifier, in_out);

    if (is_recursive_call)
        if (!socket->directly_linked_links.empty())
            return;

    std::map<NodeSocket*, NodeSocket*>* mapping = nullptr;
    if (in_out == PinKind::Input) {
        mapping = &input_mapping_from_interface_to_internal;
    }
    else {
        mapping = &output_mapping_from_interface_to_internal;
    }
    if (!mapping->contains(socket))
        return;

    NodeSocket* internal_socket = mapping->at(socket);

    if (!internal_socket->directly_linked_links.empty())
        return;
    mapping->erase(socket);

    auto group = std::find_if(
        socket_groups.begin(),
        socket_groups.end(),
        [&group_identifier, in_out](const auto& group) {
            return group->identifier == group_identifier &&
                   group->kind == in_out;
        });

    if (group == socket_groups.end()) {
        throw std::runtime_error("Socket group not found.");
    }
    (*group)->remove_socket(identifier);

    if (!is_recursive_call) {
        if (in_out == PinKind::Input)
            group_in->group_remove_socket(
                InsideOutputsPH,
                internal_socket->identifier,
                PinKind::Output,
                true);
        else
            group_out->group_remove_socket(
                InsideInputsPH,
                internal_socket->identifier,
                PinKind::Input,
                true);
    }
}

void NodeGroup::deserialize(const nlohmann::json& node_json)
{
    Node::deserialize(node_json);
    group_in = sub_tree->find_node(NODE_GROUP_IN_IDENTIFIER);
    group_out = sub_tree->find_node(NODE_GROUP_OUT_IDENTIFIER);

    sub_tree->parent_node = this;

    for (int i = 0; i < inputs.size(); ++i) {
        auto input = inputs[i];
        if (input->is_placeholder())
            continue;

        input_mapping_from_interface_to_internal[input] =
            group_in->get_outputs()[i];
    }

    for (int i = 0; i < outputs.size(); ++i) {
        auto output = outputs[i];
        if (output->is_placeholder())
            continue;
        output_mapping_from_interface_to_internal[output] =
            group_out->get_inputs()[i];
    }
}

std::pair<NodeSocket*, NodeSocket*> NodeGroup::node_group_add_input_socket(
    const char* type_name,
    const char* identifier,
    const char* name)
{
    auto added_outside_socket = Node::group_add_socket(
        OutsideInputsPH,
        type_name,
        (identifier + std::to_string(tree_->UniqueID())).c_str(),
        name,
        PinKind::Input);
    auto added_internal_socket = group_in->group_add_socket(
        InsideOutputsPH,
        type_name,
        (identifier + std::to_string(tree_->UniqueID())).c_str(),
        name,
        PinKind::Output);

    input_mapping_from_interface_to_internal[added_outside_socket] =
        added_internal_socket;

    return std::pair(added_outside_socket, added_internal_socket);
}

std::pair<NodeSocket*, NodeSocket*> NodeGroup::node_group_add_output_socket(
    const char* type_name,
    const char* identifier,
    const char* name)
{
    auto added_outside_socket = Node::group_add_socket(
        OutsideOutputsPH,
        type_name,
        (identifier + std::to_string(tree_->UniqueID())).c_str(),
        name,
        PinKind::Output);
    auto added_internal_socket = group_out->group_add_socket(
        InsideInputsPH,
        type_name,
        (identifier + std::to_string(tree_->UniqueID())).c_str(),
        name,
        PinKind::Input);

    output_mapping_from_interface_to_internal[added_outside_socket] =
        added_internal_socket;

    return std::pair(added_outside_socket, added_internal_socket);
}

SocketGroup* SocketGroupDeclaration::build(NodeTree* ntree, Node* node) const
{
    SocketGroup* group = new SocketGroup();
    group->node = node;
    group->kind = in_out;
    group->identifier = identifier;
    group->runtime_dynamic = runtime_dynamic;
    group->type_info = type;

    if (runtime_dynamic) {
        group->add_socket(get_type_name(type).c_str(), identifier.c_str(), "");
    }

    return group;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
