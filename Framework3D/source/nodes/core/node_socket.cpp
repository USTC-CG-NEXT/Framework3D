#include "node_socket.hpp"

#include "all_socket_types.hpp"
#include "node.hpp"
#include "node_register.h"
//#include "socket_type_aliases.hpp"
//#include "RCore/Backend.hpp"
#include "USTC_CG.h"
#include "Macro/map.h"
#include "entt/meta/resolve.hpp"
// #include "rich_type_buffer.hpp"
#if USTC_CG_WITH_TORCH
#include "torch/torch.h"
#endif

USTC_CG_NAMESPACE_OPEN_SCOPE

static std::map<std::string, std::unique_ptr<SocketTypeInfo>> socket_registry;

#define TYPE_NAME(CASE) \
    case SocketType::CASE: return "Socket" #CASE;

const char* get_socket_typename(SocketType socket)
{
    switch (socket) {
        MACRO_MAP(TYPE_NAME, ALL_SOCKET_TYPES)
        default: return "";
    }
}

#define TYPE_STRING(CASE) \
    case SocketType::CASE: return #CASE;

const char* get_socket_name_string(SocketType socket)
{
    switch (socket) {
        MACRO_MAP(TYPE_STRING, ALL_SOCKET_TYPES)
        default: return "";
    }
}
extern std::map<std::string, NodeTypeInfo*> conversion_node_registry;

SocketTypeInfo* make_standard_socket_type(SocketType socket)
{
    auto type_info = new SocketTypeInfo();
    type_info->type = socket;
    strcpy(type_info->type_name, get_socket_typename(socket));

    for (auto&& node_registry : conversion_node_registry) {
        if (node_registry.second->conversion_from == socket) {
            type_info->conversionTo.emplace(
                node_registry.second->conversion_to);
        }
    }

    return type_info;
}

static SocketTypeInfo* make_socket_type_Any()
{
    SocketTypeInfo* socket_type = make_standard_socket_type(SocketType::Any);
    // socket_type->cpp_type = entt::resolve<entt::meta_any>();
    return socket_type;
}
#define MAKE_SOCKET_TYPE(CASE)                                         \
    static SocketTypeInfo* make_socket_type_##CASE()                   \
    {                                                                  \
        SocketTypeInfo* socket_type =                                  \
            make_standard_socket_type(SocketType::CASE);               \
        socket_type->cpp_type = entt::resolve<socket_aliases::CASE>(); \
        return socket_type;                                            \
    }

//MACRO_MAP(MAKE_SOCKET_TYPE, ALL_SOCKET_TYPES_EXCEPT_ANY)

void register_socket(SocketTypeInfo* type_info)
{
    socket_registry[type_info->type_name] =
        std::unique_ptr<SocketTypeInfo>(type_info);
}

//void register_sockets()
//{
//#define REGISTER_NODE(NAME) register_socket(make_socket_type_##NAME());
//
//    MACRO_MAP(REGISTER_NODE, ALL_SOCKET_TYPES)
//}
SocketTypeInfo* socketTypeFind(const char* idname)
{
    if (idname[0]) {
        auto& registry = socket_registry;
        auto nt = registry.at(std::string(idname)).get();
        if (nt) {
            return nt;
        }
    }

    return nullptr;
}

bool SocketTypeInfo::canConvertTo(SocketType other) const
{
    if (type == SocketType::Any) {
        return true;
    }
    if (other == SocketType::Any) {
        return true;
    }

    if (!conversionNode(other).empty()) {
        return true;
    }
    return type == other;
}

void NodeSocket::Serialize(nlohmann::json& value)
{
    auto& socket = value[std::to_string(ID.Get())];
    // Repeated storage. Simpler code for iteration.
    socket["ID"] = ID.Get();
    socket["id_name"] = type_info->type_name;
    socket["identifier"] = identifier;
    socket["ui_name"] = ui_name;
    socket["in_out"] = in_out;

    if (dataField.value) {
        switch (type_info->type) {
            case SocketType::Int:
                socket["value"] = default_value_typed<int>();
                break;
            case SocketType::Float:
                socket["value"] = default_value_typed<float>();
                break;
            case SocketType::String:
                socket["value"] = default_value_typed<std::string&>().c_str();
                break;
            default: break;
        }
    }
}

void NodeSocket::DeserializeInfo(nlohmann::json& socket_json)
{
    ID = socket_json["ID"].get<unsigned>();

    type_info =
        socketTypeFind(socket_json["id_name"].get<std::string>().c_str());
    in_out = socket_json["in_out"].get<PinKind>();
    strcpy(ui_name, socket_json["ui_name"].get<std::string>().c_str());
    strcpy(identifier, socket_json["identifier"].get<std::string>().c_str());
}

void NodeSocket::DeserializeValue(const nlohmann::json& value)
{
    if (dataField.value) {
        if (value.find("value") != value.end()) {
            switch (type_info->type) {
                case SocketType::Int:
                    default_value_typed<int&>() = value["value"];
                    break;
                case SocketType::Float:
                    default_value_typed<float&>() = value["value"];
                    break;
                case SocketType::String: {
                    std::string str = value["value"];
                    strcpy(
                        default_value_typed<std::string&>().data(),
                        str.c_str());
                } break;
                default: break;
            }
        }
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
