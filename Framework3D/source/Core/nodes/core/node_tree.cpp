#include "nodes/core/node_tree.hpp"

#include <iostream>
#include <stack>

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

unsigned NodeTree::UniqueID()
{
    while (used_ids.find(current_id) != used_ids.end()) {
        current_id++;
    }

    return current_id++;
}

NodeLink* NodeTree::add_link(NodeSocket* fromsock, NodeSocket* tosock)
{
    SetDirty(true);

    auto fromnode = fromsock->node;
    auto tonode = tosock->node;

    if (fromsock->is_placeholder()) {
        fromsock = fromnode->group_add_socket(
            fromsock->socket_group_identifier,
            get_type_name(tosock->type_info).c_str(),
            (tosock->identifier + std::to_string(long long(tosock))).c_str(),
            tosock->ui_name,
            fromsock->in_out);
    }

    if (tosock->is_placeholder()) {
        tosock = tonode->group_add_socket(
            tosock->socket_group_identifier,
            get_type_name(fromsock->type_info).c_str(),
            (fromsock->identifier + std::to_string(long long(fromsock)))
                .c_str(),
            fromsock->ui_name,
            tosock->in_out);
    }

    if (fromsock->in_out == PinKind::Input) {
        std::swap(fromnode, tonode);
        std::swap(fromsock, tosock);
    }

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

void NodeTree::delete_link(LinkId linkId, bool refresh_topology)
{
    SetDirty(true);

    auto link = std::find_if(links.begin(), links.end(), [linkId](auto& link) {
        return link->ID == linkId;
    });
    if (link != links.end()) {
        auto group = (*link)->from_sock->socket_group;
        if (group && ((*link)->from_sock->directly_linked_links.size() == 1)) {
            group->remove_socket((*link)->from_sock);
        }

        group = (*link)->to_sock->socket_group;
        if (group && ((*link)->to_sock->directly_linked_links.size() == 1)) {
            group->remove_socket((*link)->to_sock);
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

void NodeTree::delete_link(NodeLink* link, bool refresh_topology)
{
    delete_link(link->ID, refresh_topology);
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

    // Remove the links connected to the socket

    auto& directly_connect_links = (*id)->directly_linked_links;
    for (auto& link : directly_connect_links) {
        delete_link(link->ID, false);
    }

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

    assert(tree_runtime.nodes.size() == r_sorted_nodes.size());
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

std::string NodeTree::serialize(int indentation)
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

void NodeTree::Deserialize(const std::string& str)
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

        auto node = std::make_unique<Node>(this, id, id_name.c_str());

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
