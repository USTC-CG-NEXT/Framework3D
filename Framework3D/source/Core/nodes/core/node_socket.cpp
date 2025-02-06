

#include "Logger/Logger.h"
#include "nodes/core/api.h"
#include "nodes/core/api.hpp"
#include "nodes/core/node.hpp"
#include "nodes/core/node_tree.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

extern std::map<std::string, NodeTypeInfo*> conversion_node_registry;

void NodeSocket::Serialize(nlohmann::json& value)
{
    auto& socket = value[std::to_string(ID.Get())];

    // Socket group treatment
    if (!socket_group_identifier.empty() && !std::string(ui_name).empty())
        socket["socket_group_identifier"] = socket_group_identifier;

    // Repeated storage. Simpler code for iteration.
    socket["ID"] = ID.Get();
    socket["id_name"] = get_type_name(type_info);
    socket["identifier"] = identifier;
    socket["ui_name"] = ui_name;
    socket["in_out"] = in_out;

    if (dataField.value) {
        switch (type_info.id()) {
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
            case entt::type_hash<bool>().value():
                socket["value"] = default_value_typed<bool>();
                break;
            default: log::error("Unknown type in serialization"); break;
        }
    }
}

void NodeSocket::DeserializeInfo(nlohmann::json& socket_json)
{
    ID = socket_json["ID"].get<unsigned>();

    type_info =
        get_socket_type(socket_json["id_name"].get<std::string>().c_str());
    // socketTypeFind(socket_json["id_name"].get<std::string>().c_str());
    in_out = socket_json["in_out"].get<PinKind>();
    strcpy(ui_name, socket_json["ui_name"].get<std::string>().c_str());
    strcpy(identifier, socket_json["identifier"].get<std::string>().c_str());

    if (socket_json.find("socket_group_identifier") != socket_json.end()) {
        socket_group_identifier =
            socket_json["socket_group_identifier"].get<std::string>();
    }
}

void NodeSocket::DeserializeValue(const nlohmann::json& value)
{
    if (dataField.value) {
        if (value.find("value") != value.end()) {
            switch (type_info.id()) {
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
                case entt::type_hash<bool>():
                    default_value_typed<bool&>() = value["value"];
                    break;
                default: break;
            }
        }
    }
}

NodeSocket* SocketGroup::add_socket(
    const char* type_name,
    const char* socket_identifier,
    const char* name)
{
    assert(!std::string(identifier).empty());

    auto socket = node->add_socket(type_name, socket_identifier, name, kind);
    socket->socket_group = this;
    socket->socket_group_identifier = identifier;

    if (!std::string(name).empty())
        sockets.insert(sockets.end() - 1, socket);
    else
        sockets.push_back(socket);

    return socket;
}

void SocketGroup::remove_socket(const char* socket_identifier)
{
    auto it = std::find_if(
        sockets.begin(),
        sockets.end(),
        [socket_identifier](NodeSocket* socket) {
            return strcmp(socket->identifier, socket_identifier) == 0;
        });

    if (it != sockets.end()) {
        sockets.erase(it);
    }
    node->refresh_node();
}

void SocketGroup::remove_socket(NodeSocket* socket)
{
    auto it = std::find(sockets.begin(), sockets.end(), socket);
    if (it != sockets.end()) {
        sockets.erase(it);
    }
    node->refresh_node();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
