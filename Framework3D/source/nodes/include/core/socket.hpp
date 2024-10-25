#pragma once

#include <string>
#include <unordered_set>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct SocketType {

    SocketType()
    {
    }

    static std::unique_ptr<SocketType> get_socket_type(const char* type_name);

    std::string type_name() const;

    bool operator==(const SocketType&) const = default;

    // Conversion
    bool canConvertTo(const SocketType& other) const;
    std::string conversionNode(const SocketType& to_type) const;

    entt::meta_type cpp_type;
    std::unordered_set<entt::hashed_string::value_type> conversionTo;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE