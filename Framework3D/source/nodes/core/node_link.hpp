#pragma once

#include "USTC_CG.h"
#include "id.hpp"
#include "io/json.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct NodeLink {
    LinkId ID;

    Node* from_node = nullptr;
    Node* to_node = nullptr;
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