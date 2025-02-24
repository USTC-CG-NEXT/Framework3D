#pragma once

#include "nodes/core/api.h"
#include "nodes/core/id.hpp"
#include "nodes/core/io/json.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct Node;
struct NodeSocket;
struct NODES_CORE_API NodeLink {
    LinkId ID;

    Node* from_node = nullptr;
    Node* to_node = nullptr;

    NodeSocket* get_logical_from_socket()
    {
        if (fromLink) {
            return fromLink->get_logical_from_socket();
        }
        return from_sock;
    }

    NodeSocket* get_logical_to_socket()
    {
        if (nextLink) {
            return nextLink->get_logical_to_socket();
        }
        return to_sock;
    }

    Node* get_conversion_node()
    {
        if (nextLink) {
            return nextLink->from_node;
        }
        return nullptr;
    }

    NodeSocket* from_sock = nullptr;
    NodeSocket* to_sock = nullptr;

    SocketID StartPinID;
    SocketID EndPinID;

    // Used for invisible nodes when conversion
    NodeLink* fromLink = nullptr;
    NodeLink* nextLink = nullptr;

    NodeLink(LinkId id, SocketID startPinId, SocketID endPinId)
        : ID(id),
          StartPinID(startPinId),
          EndPinID(endPinId)
    {
    }

    void Serialize(nlohmann::json& value);
};

USTC_CG_NAMESPACE_CLOSE_SCOPE