#include <string>

#include "basic_node_base.h"
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

struct Storage {
    static constexpr bool has_storage = true;

    static std::map<std::string, entt::meta_any> storage;

    nlohmann::json serialize() const
    {
        nlohmann::json j;
        for (const auto& [key, value] : storage) {
            // Assuming value can be cast to a type that nlohmann::json can
            // handle
            switch (value.type().id()) {
                case entt::type_hash<int>().value():
                    j[key] = value.cast<int>();
                    break;
                case entt::type_hash<float>().value():
                    j[key] = value.cast<float>();
                    break;
                case entt::type_hash<std::string>().value():
                    j[key] = value.cast<std::string>();
                    break;
                default:
                    // Do nothing
                    std::cout << "Type " << value.type().id()
                              << " not supported" << std::endl;
                    break;
            }
        }
        return j;
    }

    void deserialize(const nlohmann::json& j)
    {
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_number_integer()) {
                storage[it.key()] = it.value().get<int>();
            }
            else if (it.value().is_number_float()) {
                storage[it.key()] = it.value().get<double>();
            }
            else if (it.value().is_string()) {
                storage[it.key()] = it.value().get<std::string>();
            }
            else {
                // Do nothing
                std::cout << "Type not supported" << std::endl;
            }
        }
    }
};

std::map<std::string, entt::meta_any> Storage::storage;

NODE_DECLARATION_FUNCTION(storage_in)
{
    b.add_input<std::string>("Name").default_val("Storage");
    b.add_input<entt::meta_any>("storage");
}

NODE_EXECUTION_FUNCTION(storage_in)
{
    auto& s = params.get_storage<Storage&>();
    auto name = params.get_input<std::string>("Name");
    name = std::string(name.c_str());
    auto storage = params.get_input<entt::meta_any>("storage");

    s.storage[name] = storage;

    return true;
}

NODE_DECLARATION_FUNCTION(storage_out)
{
    b.add_input<std::string>("Name").default_val("Storage");
    b.add_output<entt::meta_any>("storage");
}

NODE_EXECUTION_FUNCTION(storage_out)
{
    auto& s = params.get_storage<Storage&>();
    auto name = params.get_input<std::string>("Name");
    name = std::string(name.c_str());

    if (s.storage.find(name) != s.storage.end()) {
        auto storage = s.storage[name];
        params.set_output("storage", storage);
        return true;
    }

    return false;
}

NODE_DECLARATION_FUNCTION(storage_load_and_save_json)
{
}

NODE_EXECUTION_FUNCTION(storage_load_and_save_json)
{
    auto& s = params.get_storage<Storage&>();
    params.set_storage(s);

    return true;
}

NODE_DECLARATION_FUNCTION(storage_clear)
{
}

NODE_EXECUTION_FUNCTION(storage_clear)
{
    auto& s = params.get_storage<Storage&>();
    s.storage.clear();
    return true;
}

NODE_DECLARATION_UI(storage);
NODE_DEF_CLOSE_SCOPE
