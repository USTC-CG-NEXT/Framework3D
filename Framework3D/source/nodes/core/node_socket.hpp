#pragma once

#include "USTC_CG.h"
#include "entt/meta/meta.hpp"
#include "id.hpp"
#include "io/json.hpp"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
#define TypeSizeEnum(Type, Size) Type##Size##Buffer
// enum class SocketType : uint32_t { ALL_SOCKET_TYPES };
#undef TypeSizeEnum

// const char* get_socket_name_string(SocketType socket);

enum class NodeType {
    Blueprint,
    NodeGroup,
    GroupIn,
    GroupOut,
    Simple,
    Comment,
};

struct Node;
struct NodeLink;

USTC_CG_NAMESPACE_CLOSE_SCOPE
