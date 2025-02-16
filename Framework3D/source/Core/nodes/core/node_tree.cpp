#include "nodes/core/node_tree.hpp"

#include <iostream>
#include <set>
#include <stack>
#include <unordered_set>

#include "nodes/core/io/json.hpp"
#include "nodes/core/node.hpp"
#include "nodes/core/node_link.hpp"

// Macro for Not implemented with file and line number
#define NOT_IMPLEMENTED()                                               \
    do {                                                                \
        std::cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ \
                  << std::endl;                                         \
        std::abort();                                                   \
    } while (0)

USTC_CG_NAMESPACE_OPEN_SCOPE
NodeTreeDescriptor::NodeTreeDescriptor()
{
    register_node(
        NodeTypeInfo("node_group")
            .set_ui_name("Group")
            .set_declare_function([](NodeDeclarationBuilder& b) {
                b.add_input_group("Inputs");
                b.add_output_group("Outputs");
            })
            .set_execution_function([](ExeParams params) { return true; }));

    register_node(
        NodeTypeInfo("node_group_in")
            .set_ui_name("Group In")
            .set_declare_function([](NodeDeclarationBuilder& b) {
                b.add_output_group("Outputs");
            })
            .set_execution_function([](ExeParams params) { return true; }));

    register_node(
        NodeTypeInfo("node_group_out")
            .set_ui_name("Group Out")
            .set_declare_function(
                [](NodeDeclarationBuilder& b) { b.add_input_group("Inputs"); })
            .set_execution_function([](ExeParams params) { return true; }));
}

NodeTreeDescriptor::~NodeTreeDescriptor()
{
}

NodeTreeDescriptor& NodeTreeDescriptor::register_node(
    const NodeTypeInfo& type_info)
{
    node_registry[type_info.id_name] = type_info;
    return *this;
}

NodeTreeDescriptor& NodeTreeDescriptor::register_conversion_name(
    const std::string& conversion_name)
{
    conversion_node_registry.insert(conversion_name);
    return *this;
}

const NodeTypeInfo* NodeTreeDescriptor::get_node_type(
    const std::string& name) const
{
    auto it = node_registry.find(name);
    if (it != node_registry.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string NodeTreeDescriptor::conversion_node_name(
    SocketType from,
    SocketType to)
{
    return std::string("conv_") + get_type_name(from) + "_to_" +
           get_type_name(to);
}

bool NodeTreeDescriptor::can_convert(SocketType from, SocketType to) const
{
    if (!from || !to) {
        return true;
    }
    auto node_name = conversion_node_name(from, to);
    return conversion_node_registry.find(node_name) !=
           conversion_node_registry.end();
}

NodeTree::NodeTree(const NodeTreeDescriptor& descriptor)
    : has_available_link_cycle(false),
      descriptor_(descriptor)
{
    links.reserve(32);
    sockets.reserve(32);
    nodes.reserve(32);
    input_sockets.reserve(32);
    output_sockets.reserve(32);
    toposort_right_to_left.reserve(32);
    toposort_left_to_right.reserve(32);
}

NodeTree::NodeTree(const NodeTree& other)
{
    // A deep copy by reconstructing the tree
    deserialize(other.serialize());
}

NodeTree& NodeTree::operator=(const NodeTree& other)
{
    if (this == &other) {
        return *this;
    }
    clear();
    deserialize(other.serialize());
    return *this;
}

NodeTree::~NodeTree()
{
}
unsigned NodeTree::input_socket_id(NodeSocket* socket)
{
    return std::distance(
        input_sockets.begin(),
        std::find(input_sockets.begin(), input_sockets.end(), socket));
}

unsigned NodeTree::output_socket_id(NodeSocket* socket)
{
    return std::distance(
        output_sockets.begin(),
        std::find(output_sockets.begin(), output_sockets.end(), socket));
}

const std::vector<Node*>& NodeTree::get_toposort_right_to_left() const
{
    return toposort_right_to_left;
}

const std::vector<Node*>& NodeTree::get_toposort_left_to_right() const
{
    return toposort_left_to_right;
}

size_t NodeTree::socket_count() const
{
    return sockets.size();
}

const NodeTreeDescriptor& NodeTree::get_descriptor() const
{
    return descriptor_;
}

void NodeTree::SetDirty(bool dirty)
{
    this->dirty_ = dirty;
}

bool NodeTree::GetDirty()
{
    return dirty_;
}

void NodeTree::clear()
{
    links.clear();
    sockets.clear();
    nodes.clear();
    input_sockets.clear();
    output_sockets.clear();
    toposort_right_to_left.clear();
    toposort_left_to_right.clear();
}

Node* NodeTree::find_node(NodeId id) const
{
    if (!id)
        return nullptr;

    for (auto& node : nodes) {
        if (node->ID == id) {
            return node.get();
        }
    }
    return nullptr;
}

NodeSocket* NodeTree::find_pin(SocketID id) const
{
    if (!id)
        return nullptr;

    for (auto& socket : sockets) {
        if (socket->ID == id) {
            return socket.get();
        }
    }
    return nullptr;
}

NodeLink* NodeTree::find_link(LinkId id) const
{
    if (!id)
        return nullptr;

    for (auto& link : links) {
        if (link->ID == id) {
            return link.get();
        }
    }

    return nullptr;
}

bool NodeTree::is_pin_linked(SocketID id) const
{
    return !find_pin(id)->directly_linked_links.empty();
}

Node* NodeTree::add_node(const char* idname)
{
    auto node = std::make_unique<Node>(this, idname);
    auto bare = node.get();
    nodes.push_back(std::move(node));
    bare->refresh_node();
    return bare;
}

void NodeTree::add_base_id(unsigned max_used_id)
{
    for (auto& node : nodes) {
        node->ID += max_used_id;
    }

    for (auto& socket : sockets) {
        socket->ID += max_used_id;
    }
    for (auto& link : links) {
        link->ID += max_used_id;
        link->StartPinID += max_used_id;
        link->EndPinID += max_used_id;
    }
}

NodeTree& NodeTree::merge(const NodeTree& other)
{
    auto copy_other = other;
    merge(std::move(copy_other));
    return *this;
}

NodeTree& NodeTree::merge(NodeTree&& other)
{
    auto max_used_id = current_id;
    other.add_base_id(max_used_id);
    nodes.insert(
        nodes.end(),
        std::make_move_iterator(other.nodes.begin()),
        std::make_move_iterator(other.nodes.end()));
    links.insert(
        links.end(),
        std::make_move_iterator(other.links.begin()),
        std::make_move_iterator(other.links.end()));
    sockets.insert(
        sockets.end(),
        std::make_move_iterator(other.sockets.begin()),
        std::make_move_iterator(other.sockets.end()));
    return *this;
}

template<
    typename NodePtrContainer,
    typename NodeLinkPtrContainer,
    typename NodeSocketPtrContainer>
std::string tree_serialize(
    const NodePtrContainer& nodes,
    const NodeLinkPtrContainer& links,
    const NodeSocketPtrContainer& sockets,
    int indentation = -1)
{
    nlohmann::json value;

    auto& node_info = value["nodes_info"];
    for (auto&& node : nodes) {
        node->serialize(node_info);
    }

    auto& links_info = value["links_info"];
    for (auto&& link : links) {
        link->Serialize(links_info);
    }

    auto& sockets_info = value["sockets_info"];
    for (auto&& socket : sockets) {
        socket->Serialize(sockets_info);
    }

    std::ostringstream s;
    s << value.dump(indentation);
    return s.str();
}

// remembder to adopt the node!
static NodeGroup* create_group_node(NodeTree* tree)
{
    NodeGroup* node = new NodeGroup(tree, "node_group");
    tree->nodes.push_back(std::unique_ptr<Node>(node));
    return node;
}

static Node* create_group_node_in(NodeTree* tree)
{
    Node* node = new Node(tree, "node_group_in");
    tree->nodes.push_back(std::unique_ptr<Node>(node));
    return node;
}

static Node* create_group_node_out(NodeTree* tree)
{
    Node* node = new Node(tree, "node_group_out");
    tree->nodes.push_back(std::unique_ptr<Node>(node));
    return node;
}

NodeGroup* NodeTree::group_up(const std::vector<Node*>& nodes_to_group)
{
    auto sockets_to_group = std::set<NodeSocket*>();

    for (auto& node : nodes_to_group) {
        for (auto& socket : node->get_inputs()) {
            sockets_to_group.insert(socket);
        }

        for (auto& socket : node->get_outputs()) {
            sockets_to_group.insert(socket);
        }
    }

    auto links_to_group = std::set<NodeLink*>();

    // find the links between the sockets to group

    for (auto& link : links) {
        if (sockets_to_group.find(link->from_sock) != sockets_to_group.end() &&
            sockets_to_group.find(link->to_sock) != sockets_to_group.end()) {
            links_to_group.insert(link.get());
        }
    }

    // create a new group node
    auto group_node = create_group_node(this);
    auto serialized =
        tree_serialize(nodes_to_group, links_to_group, sockets_to_group, 2);
    group_node->sub_tree->deserialize(serialized);

    group_node->group_in = create_group_node_in(group_node->sub_tree.get());
    group_node->group_out = create_group_node_out(group_node->sub_tree.get());

    group_node->sub_tree->ensure_topology_cache();

    // find the group upstream, i.e. any links that is connected to the
    // sockets_to_group, but not in the links_to_group

    auto upstream_links = std::set<NodeLink*>();
    auto downstream_links = std::set<NodeLink*>();

    for (auto& socket : sockets_to_group) {
        for (auto& link : socket->directly_linked_links) {
            if (links_to_group.find(link) == links_to_group.end()) {
                if (socket->in_out == PinKind::Input) {
                    upstream_links.insert(link);
                }
                else {
                    downstream_links.insert(link);
                }
            }
        }
    }

    for (auto& link : upstream_links) {
        auto from_socket = link->from_sock;
        auto to_socket = link->to_sock;

        auto [added_outside_socket, added_internal_socket] =
            group_node->node_group_add_input_socket(
                get_type_name(to_socket->type_info).c_str(),
                to_socket->identifier,
                to_socket->ui_name);

        add_link(from_socket, added_outside_socket);
        group_node->sub_tree->add_link(
            added_internal_socket,
            group_node->sub_tree->find_pin(link->EndPinID));
    }

    for (auto& link : downstream_links) {
        auto from_socket = link->from_sock;
        auto to_socket = link->to_sock;

        auto [added_outside_socket, added_internal_socket] =
            group_node->node_group_add_output_socket(
                get_type_name(from_socket->type_info).c_str(),
                from_socket->identifier,
                from_socket->ui_name);

        add_link(added_outside_socket, to_socket, true);
        group_node->sub_tree->add_link(
            group_node->sub_tree->find_pin(link->StartPinID),
            added_internal_socket);
    }

    // remove nodes_to_group
    for (auto& node : nodes_to_group) {
        delete_node(node);
    }

    return group_node;
}

void NodeTree::ungroup(Node* node)
{
    assert(node->typeinfo->id_name == "node_group");

    auto serialized = static_cast<NodeGroup*>(node)->sub_tree->serialize();
    auto deserialized = std::make_unique<NodeTree>(descriptor_);
    deserialized->deserialize(serialized);
}

unsigned NodeTree::UniqueID()
{
    while (used_ids.find(current_id) != used_ids.end()) {
        current_id++;
    }

    return current_id++;
}

NodeLink* NodeTree::add_link(
    NodeSocket* fromsock,
    NodeSocket* tosock,
    bool allow_relink_to_output)
{
    SetDirty(true);

    auto fromnode = fromsock->node;
    auto tonode = tosock->node;

    if (fromsock->is_placeholder()) {
        fromsock = fromnode->group_add_socket(
            fromsock->socket_group_identifier,
            get_type_name(tosock->type_info).c_str(),
            (tosock->identifier + std::to_string(UniqueID())).c_str(),
            tosock->ui_name,
            fromsock->in_out);
    }

    if (tosock->is_placeholder()) {
        tosock = tonode->group_add_socket(
            tosock->socket_group_identifier,
            get_type_name(fromsock->type_info).c_str(),
            (fromsock->identifier + std::to_string(UniqueID())).c_str(),
            fromsock->ui_name,
            tosock->in_out);
    }

    if (fromsock->in_out == PinKind::Input) {
        std::swap(fromnode, tonode);
        std::swap(fromsock, tosock);
    }

    if (!allow_relink_to_output)
        if (!tosock->directly_linked_sockets.empty()) {
            throw std::runtime_error("Socket already linked.");
        }

    NodeLink* bare_ptr;

    if (fromsock->type_info == tosock->type_info || !fromsock->type_info ||
        !tosock->type_info) {
        auto link =
            std::make_unique<NodeLink>(UniqueID(), fromsock->ID, tosock->ID);

        link->from_node = fromnode;
        link->from_sock = fromsock;
        link->to_node = tonode;
        link->to_sock = tosock;
        bare_ptr = link.get();
        links.push_back(std::move(link));
    }
    else if (descriptor_.can_convert(fromsock->type_info, tosock->type_info)) {
        std::string conversion_node_name;

        conversion_node_name = descriptor_.conversion_node_name(
            fromsock->type_info, tosock->type_info);

        auto middle_node = add_node(conversion_node_name.c_str());
        assert(middle_node->get_inputs().size() == 1);
        assert(middle_node->get_outputs().size() == 1);

        auto middle_tosock = middle_node->get_inputs()[0];
        auto middle_fromsock = middle_node->get_outputs()[0];

        auto firstLink = add_link(fromsock, middle_tosock);

        auto nextLink = add_link(middle_fromsock, tosock);
        assert(firstLink);
        assert(nextLink);
        firstLink->nextLink = nextLink;
        nextLink->fromLink = firstLink;
        bare_ptr = firstLink;
    }
    else {
        throw std::runtime_error("Cannot convert between types.");
    }
    ensure_topology_cache();
    return bare_ptr;
}

NodeLink* NodeTree::add_link(SocketID startPinId, SocketID endPinId)
{
    SetDirty(true);
    auto socket1 = find_pin(startPinId);
    auto socket2 = find_pin(endPinId);

    if (socket1 && socket2)
        return add_link(socket1, socket2);
}

void NodeTree::delete_link(
    LinkId linkId,
    bool refresh_topology,
    bool remove_from_group)
{
    SetDirty(true);

    auto link = std::find_if(links.begin(), links.end(), [linkId](auto& link) {
        return link->ID == linkId;
    });
    if (link != links.end()) {
        if (remove_from_group) {
            auto group = (*link)->from_sock->socket_group;
            if (group &&
                ((*link)->from_sock->directly_linked_links.size() == 1)) {
                group->remove_socket((*link)->from_sock);
            }
            group = (*link)->to_sock->socket_group;
            if (group &&
                ((*link)->to_sock->directly_linked_links.size() == 1)) {
                group->remove_socket((*link)->to_sock);
            }
        }

        if ((*link)->nextLink) {
            auto nextLink = (*link)->nextLink;

            auto conversion_node = (*link)->to_node;

            links.erase(link);

            // Find next link iterator
            auto nextLinkIter = std::find_if(
                links.begin(), links.end(), [nextLink](auto& link) {
                    return link.get() == nextLink;
                });

            links.erase(nextLinkIter);

            delete_node(conversion_node);
        }
        else {
            links.erase(link);
        }
    }
    if (refresh_topology) {
        ensure_topology_cache();
    }
}

void NodeTree::delete_link(
    NodeLink* link,
    bool refresh_topology,
    bool remove_from_group)
{
    delete_link(link->ID, refresh_topology, remove_from_group);
}

void NodeTree::delete_node(Node* nodeId)
{
    delete_node(nodeId->ID);
}

void NodeTree::delete_node(NodeId nodeId)
{
    auto id = std::find_if(nodes.begin(), nodes.end(), [nodeId](auto&& node) {
        return node->ID == nodeId;
    });
    if (id != nodes.end()) {
        for (auto& socket : (*id)->get_inputs()) {
            delete_socket(socket->ID);
        }

        for (auto& socket : (*id)->get_outputs()) {
            delete_socket(socket->ID);
        }

        nodes.erase(id);
    }
    ensure_topology_cache();
}

bool NodeTree::can_create_link(NodeSocket* a, NodeSocket* b)
{
    if (!a || !b || a == b || a->in_out == b->in_out || a->node == b->node)
        return false;

    if (a->is_placeholder() && b->is_placeholder()) {
        return false;
    }

    auto in = a->in_out == PinKind::Input ? a : b;
    auto out = a->in_out == PinKind::Output ? a : b;

    if (!in->directly_linked_sockets.empty()) {
        return false;
    }
    if (can_create_direct_link(out, in)) {
        return true;
    }

    if (can_create_convert_link(out, in)) {
        return true;
    }

    return false;
}

bool NodeTree::can_create_direct_link(NodeSocket* socket1, NodeSocket* socket2)
{
    return socket1->type_info == socket2->type_info;
}

bool NodeTree::can_create_convert_link(NodeSocket* out, NodeSocket* in)
{
    return descriptor_.can_convert(out->type_info, in->type_info);
}

void NodeTree::delete_socket(SocketID socketId)
{
    auto id =
        std::find_if(sockets.begin(), sockets.end(), [socketId](auto&& socket) {
            return socket->ID == socketId;
        });

    bool socket_in_group = (*id)->socket_group != nullptr;

    // Remove the links connected to the socket

    auto& directly_connect_links = (*id)->directly_linked_links;
    for (auto& link : directly_connect_links) {
        delete_link(link->ID, false, false);
    }

    if (!socket_in_group)
        if (id != sockets.end()) {
            sockets.erase(id);
        }
}

void NodeTree::update_directly_linked_links_and_sockets()
{
    for (auto&& node : nodes) {
        for (auto socket : node->get_inputs()) {
            socket->directly_linked_links.clear();
            socket->directly_linked_sockets.clear();
        }
        for (auto socket : node->get_outputs()) {
            socket->directly_linked_links.clear();
            socket->directly_linked_sockets.clear();
        }
        node->has_available_linked_inputs = false;
        node->has_available_linked_outputs = false;
    }
    for (auto&& link : links) {
        link->from_sock->directly_linked_links.push_back(link.get());
        link->from_sock->directly_linked_sockets.push_back(link->to_sock);
        link->to_sock->directly_linked_links.push_back(link.get());
        if (link) {
            link->from_node->has_available_linked_outputs = true;
            link->to_node->has_available_linked_inputs = true;
        }
    }

    for (NodeSocket* socket : input_sockets) {
        if (socket) {
            for (NodeLink* link : socket->directly_linked_links) {
                /* Do this after sorting the input links. */
                socket->directly_linked_sockets.push_back(link->from_sock);
            }
        }
    }
}

void NodeTree::update_socket_vectors_and_owner_node()
{
    input_sockets.clear();
    output_sockets.clear();
    for (auto&& socket : sockets) {
        if (socket->in_out == PinKind::Input) {
            input_sockets.push_back(socket.get());
        }
        else {
            output_sockets.push_back(socket.get());
        }
    }
}

enum class ToposortDirection {
    LeftToRight,
    RightToLeft,
};

struct ToposortNodeState {
    bool is_done = false;
    bool is_in_stack = false;
};

template<typename T>
using Vector = std::vector<T>;

static void toposort_from_start_node(
    const NodeTree& ntree,
    const ToposortDirection direction,
    Node& start_node,
    std::unordered_map<Node*, ToposortNodeState>& node_states,
    Vector<Node*>& r_sorted_nodes,
    bool& r_cycle_detected)
{
    struct Item {
        Node* node;
        int socket_index = 0;
        int link_index = 0;
        int implicit_link_index = 0;
    };

    std::stack<Item> nodes_to_check;
    nodes_to_check.push({ &start_node });
    node_states[&start_node].is_in_stack = true;
    while (!nodes_to_check.empty()) {
        Item& item = nodes_to_check.top();
        Node& node = *item.node;
        bool pushed_node = false;

        auto handle_linked_node = [&](Node& linked_node) {
            ToposortNodeState& linked_node_state = node_states[&linked_node];
            if (linked_node_state.is_done) {
                /* The linked node has already been visited. */
                return true;
            }
            if (linked_node_state.is_in_stack) {
                r_cycle_detected = true;
            }
            else {
                nodes_to_check.push({ &linked_node });
                linked_node_state.is_in_stack = true;
                pushed_node = true;
            }
            return false;
        };

        const auto& sockets = (direction == ToposortDirection::LeftToRight)
                                  ? node.get_inputs()
                                  : node.get_outputs();
        while (true) {
            if (item.socket_index == sockets.size()) {
                /* All sockets have already been visited. */
                break;
            }
            NodeSocket& socket = *sockets[item.socket_index];
            const auto& linked_links = socket.directly_linked_links;
            if (item.link_index == linked_links.size()) {
                /* All links connected to this socket have already been visited.
                 */
                item.socket_index++;
                item.link_index = 0;
                continue;
            }

            NodeSocket& linked_socket =
                *socket.directly_linked_sockets[item.link_index];
            Node& linked_node = *linked_socket.node;
            if (handle_linked_node(linked_node)) {
                /* The linked node has already been visited. */
                item.link_index++;
                continue;
            }
            break;
        }

        if (!pushed_node) {
            ToposortNodeState& node_state = node_states[&node];
            node_state.is_done = true;
            node_state.is_in_stack = false;
            r_sorted_nodes.push_back(&node);
            nodes_to_check.pop();
        }
    }
}

static void update_toposort(
    const NodeTree& ntree,
    const ToposortDirection direction,
    Vector<Node*>& r_sorted_nodes,
    bool& r_cycle_detected)
{
    const NodeTree& tree_runtime = ntree;
    r_sorted_nodes.clear();
    r_sorted_nodes.reserve(tree_runtime.nodes.size());
    r_cycle_detected = false;

    std::unordered_map<Node*, ToposortNodeState> node_states(
        tree_runtime.nodes.size());
    for (auto&& node : tree_runtime.nodes) {
        if (!node_states.contains(node.get())) {
            node_states[node.get()] = ToposortNodeState{};
        }
    }
    for (auto&& node : tree_runtime.nodes) {
        if (node_states[node.get()].is_done) {
            /* Ignore nodes that are done already. */
            continue;
        }
        if ((direction == ToposortDirection::LeftToRight)
                ? node->has_available_linked_outputs
                : node->has_available_linked_inputs) {
            /* Ignore non-start nodes. */
            continue;
        }
        toposort_from_start_node(
            ntree,
            direction,
            *node,
            node_states,
            r_sorted_nodes,
            r_cycle_detected);
    }

    if (r_sorted_nodes.size() < tree_runtime.nodes.size()) {
        r_cycle_detected = true;
        for (auto&& node : tree_runtime.nodes) {
            if (node_states[node.get()].is_done) {
                /* Ignore nodes that are done already. */
                continue;
            }
            /* Start toposort at this node which is somewhere in the middle
            of a
             * loop. */
            toposort_from_start_node(
                ntree,
                direction,
                *node,
                node_states,
                r_sorted_nodes,
                r_cycle_detected);
        }
    }

    if (tree_runtime.nodes.size() != r_sorted_nodes.size())
        throw std::runtime_error("Toposort failed");
}

void NodeTree::ensure_topology_cache()
{
    update_socket_vectors_and_owner_node();

    update_directly_linked_links_and_sockets();

    update_toposort(
        *this,
        ToposortDirection::LeftToRight,
        toposort_left_to_right,
        has_available_link_cycle);
    update_toposort(
        *this,
        ToposortDirection::RightToLeft,
        toposort_right_to_left,
        has_available_link_cycle);
}

std::string NodeTree::serialize(int indentation) const
{
    return tree_serialize(nodes, links, sockets, indentation);
}

void NodeTree::deserialize(const std::string& str)
{
    nlohmann::json value;
    std::istringstream in(str);
    in >> value;
    clear();

    // To avoid reuse of ID, push up the ID in the beginning

    for (auto&& socket_json : value["sockets_info"]) {
        used_ids.emplace(socket_json["ID"]);
    }

    for (auto&& node_json : value["nodes_info"]) {
        used_ids.emplace(node_json["ID"]);
    }

    for (auto&& link_json : value["links_info"]) {
        used_ids.emplace(link_json["ID"]);
    }

    for (auto&& socket_json : value["sockets_info"]) {
        auto socket = std::make_unique<NodeSocket>();
        socket->DeserializeInfo(socket_json);
        sockets.push_back(std::move(socket));
    }

    for (auto&& node_json : value["nodes_info"]) {
        auto id = node_json["ID"].get<unsigned>();
        auto id_name = node_json["id_name"].get<std::string>();
        auto storage_info = node_json["storage_info"];

        auto node = std::make_unique<Node>(this, id, id_name.c_str());
        node->storage_info = storage_info;

        if (!node->valid())
            continue;

        node->deserialize(node_json);
        nodes.push_back(std::move(node));
    }

    // Get the saved value in the sockets
    for (auto&& node_socket : sockets) {
        const auto& socket_value =
            value["sockets_info"][std::to_string(node_socket->ID.Get())];
        node_socket->DeserializeValue(socket_value);
    }

    for (auto&& link_json : value["links_info"]) {
        add_link(
            link_json["StartPinID"].get<unsigned>(),
            link_json["EndPinID"].get<unsigned>());
    }

    ensure_topology_cache();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
