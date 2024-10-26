#include "node_socket.hpp"

#include "all_socket_types.hpp"
#include "node.hpp"
#include "node_register.h"
// #include "socket_type_aliases.hpp"
// #include "RCore/Backend.hpp"
#include "Logging/Logging.h"
#include "Macro/map.h"
#include "USTC_CG.h"
#include "entt/meta/resolve.hpp"
// #include "rich_type_buffer.hpp"
#if USTC_CG_WITH_TORCH
#include "torch/torch.h"
#endif

USTC_CG_NAMESPACE_OPEN_SCOPE

// static std::map<std::string, std::unique_ptr<SocketType>> socket_registry;

extern std::map<std::string, NodeTypeInfo*> conversion_node_registry;

// SocketType* make_standard_socket_type(SocketType socket)
//{
//     auto type_info = new SocketType();
//     type_info->type = socket;
//     strcpy(type_info->type_name, get_socket_typename(socket));
//
//     for (auto&& node_registry : conversion_node_registry) {
//         if (node_registry.second->conversion_from == socket) {
//             type_info->conversionTo.emplace(
//                 node_registry.second->conversion_to);
//         }
//     }
//
//     return type_info;
// }
//
// static SocketType* make_socket_type_Any()
//{
//     SocketType* socket_type = make_standard_socket_type(SocketType::Any);
//     // socket_type->cpp_type = entt::resolve<entt::meta_any>();
//     return socket_type;
// }
// #def ine  MAKE _SOCKET_TYPE(CASE)                                         \
//    static SocketType* make_socket_type_##CASE()                   \
//    {                                                                  \
//        SocketType* socket_type =                                  \
//            make_standard_socket_type(SocketType::CASE);               \
//        socket_type->cpp_type = entt::resolve<socket_aliases::CASE>(); \
//        return socket_type;                                            \
//    }

// MACRO_MAP(MAKE_SOCKET_TYPE, ALL_SOCKET_TYPES_EXCEPT_ANY)

// void register_socket(SocketType* type_info)
//{
//     socket_registry[type_info->type_name()] =
//         std::unique_ptr<SocketType>(type_info);
// }

// void register_sockets()
//{
// #define REGISTER_NODE(NAME) register_socket(make_socket_type_##NAME());
//
//     MACRO_MAP(REGISTER_NODE, ALL_SOCKET_TYPES)
// }
// SocketType* socketTypeFind(const char* idname)
//{
//    if (idname[0]) {
//        auto& registry = socket_registry;
//        auto nt = registry.at(std::string(idname)).get();
//        if (nt) {
//            return nt;
//        }
//    }
//
//    return nullptr;
//}

std::unique_ptr<SocketType> SocketType::get_socket_type(const char* type_name)
{
    auto mt = entt::resolve(entt::hashed_string{ type_name });
    if (mt) {
        auto ret = std::make_unique<SocketType>();
        ret->cpp_type = mt;
        return ret;
    }
    logging("Trying to get a non-existing type with " + std::string(type_name));
    return nullptr;
}

std::string SocketType::type_name() const
{
    std::string_view name = cpp_type.info().name();
    // convert it to string and return
    return std::string(name.data(), name.size());
}

bool SocketType::canConvertTo(const SocketType& other) const
{
    // "any" can always convert to anything
    if (!cpp_type) {
        return true;
    }
    if (!other.cpp_type) {
        return true;
    }

    if (!conversionNode(other).empty()) {
        return true;
    }
    return cpp_type == other.cpp_type;
}

std::string SocketType::conversionNode(const SocketType& to_type) const
{
    if (conversionTo.contains(
            entt::hashed_string{ to_type.type_name().c_str() })) {
        return std::string("conv_") + type_name() + "_to_" +
               to_type.type_name();
    }
    return {};
}

void NodeSocket::Serialize(nlohmann::json& value)
{
    auto& socket = value[std::to_string(ID.Get())];
    // Repeated storage. Simpler code for iteration.
    socket["ID"] = ID.Get();
    socket["id_name"] = type_info->type_name();
    socket["identifier"] = identifier;
    socket["ui_name"] = ui_name;
    socket["in_out"] = in_out;

    if (dataField.value) {
        switch (type_info->cpp_type.id()) {
            using namespace entt::literals;
            case entt::type_hash<int>().value():
                socket["value"] = default_value_typed<int>();
                break;
            case entt::type_hash<float>().value():
                socket["value"] = default_value_typed<float>();
                break;
            case entt::type_hash<std::string>().value():
                socket["value"] = default_value_typed<std::string&>().c_str();
                break;
            default: logging("Unknown type in serialization"); break;
        }
    }
}

void NodeSocket::DeserializeInfo(nlohmann::json& socket_json)
{
    ID = socket_json["ID"].get<unsigned>();

    type_info = SocketType::get_socket_type(
        socket_json["id_name"].get<std::string>().c_str());
    // socketTypeFind(socket_json["id_name"].get<std::string>().c_str());
    in_out = socket_json["in_out"].get<PinKind>();
    strcpy(ui_name, socket_json["ui_name"].get<std::string>().c_str());
    strcpy(identifier, socket_json["identifier"].get<std::string>().c_str());
}

void NodeSocket::DeserializeValue(const nlohmann::json& value)
{
    if (dataField.value) {
        if (value.find("value") != value.end()) {
            switch (type_info->cpp_type.id()) {
                case entt::type_hash<int>():
                    default_value_typed<int&>() = value["value"];
                    break;
                case entt::type_hash<float>():
                    default_value_typed<float&>() = value["value"];
                    break;
                case entt::type_hash<std::string>(): {
                    std::string str = value["value"];
                    default_value_typed<std::string&>() = str;

                } break;
                default: break;
            }
        }
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
