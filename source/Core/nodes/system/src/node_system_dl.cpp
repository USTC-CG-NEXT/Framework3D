#include "nodes/system/node_system_dl.hpp"

#include <fstream>
#include <iostream>
#include <nodes/core/io/json.hpp>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
USTC_CG_NAMESPACE_OPEN_SCOPE

DynamicLibraryLoader::DynamicLibraryLoader(const std::string& libraryName)
{
#ifdef _WIN32
    handle = LoadLibrary(libraryName.c_str());
    if (!handle) {
        throw std::runtime_error("Failed to load library: " + libraryName);
    }
#else
    handle = dlopen(libraryName.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error("Failed to load library: " + libraryName);
    }
#endif
}

DynamicLibraryLoader::~DynamicLibraryLoader()
{
#ifdef _WIN32
    if (handle) {
        // FreeLibrary(handle);
    }
#else
    if (handle) {
        dlclose(handle);
    }
#endif
}

std::shared_ptr<NodeTreeDescriptor>
NodeDynamicLoadingSystem::node_tree_descriptor()
{
    return descriptor;
}

NodeDynamicLoadingSystem::NodeDynamicLoadingSystem()
{
    descriptor = std::make_shared<NodeTreeDescriptor>();
}

NodeDynamicLoadingSystem::~NodeDynamicLoadingSystem()
{
    descriptor = {};
    this->node_tree.reset();
    this->node_tree_executor.reset();

    this->node_libraries.clear();
}

bool NodeDynamicLoadingSystem::load_configuration(
    const std::filesystem::path& config_file_path)
{
    nlohmann::json j;

    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error(
            "Failed to open configuration file: " + config_file_path.string());
    }

    config_file >> j;
    config_file.close();

    auto load_libraries = [&](const nlohmann::json& json_section,
                              auto& library_map,
                              const std::string& extension,
                              bool is_conversion) {
        for (auto it = json_section.begin(); it != json_section.end(); ++it) {
            std::string key = it.key();
            auto func_names = it.value();

            library_map[key] =
                std::make_unique<DynamicLibraryLoader>(key + extension);

            for (auto&& func_name : func_names) {
                auto func_name_str = func_name.get<std::string>();
                auto node_ui_name =
                    library_map[key]->template getFunction<const char*()>(
                        "node_ui_name_" + func_name_str);

                auto node_id_name =
                    library_map[key]->template getFunction<std::string()>(
                        "node_id_name_" + func_name_str);

                auto node_always_requred =
                    library_map[key]->template getFunction<bool()>(
                        "node_required_" + func_name_str);

                auto node_declare =
                    library_map[key]
                        ->template getFunction<void(NodeDeclarationBuilder&)>(
                            "node_declare_" + func_name_str);
                auto node_execution =
                    library_map[key]->template getFunction<bool(ExeParams)>(
                        "node_execution_" + func_name_str);

                NodeTypeInfo new_node;

                if (is_conversion) {
                    new_node.id_name =
                        node_id_name();  // For a conversion node, id name must
                                         // exist.
                    new_node.ui_name = "invisible";
                    new_node.INVISIBLE = true;
                    descriptor->register_conversion_name(node_id_name());
                }
                else {
                    new_node.id_name =
                        node_id_name ? node_id_name() : func_name_str;
                    new_node.ui_name =
                        node_ui_name ? node_ui_name() : new_node.id_name;
                }

                new_node.ALWAYS_REQUIRED =
                    node_always_requred ? node_always_requred() : false;
                if (new_node.ALWAYS_REQUIRED) {
                    log::info("%s is always required.", func_name_str.c_str());
                }
                new_node.set_declare_function(node_declare);
                new_node.set_execution_function(node_execution);

                descriptor->register_node(new_node);
            }
        }
    };

#ifdef _WIN32
    std::string extension = ".dll";
#else
    std::string extension = ".so";
#endif

    load_libraries(j["nodes"], node_libraries, extension, false);
    load_libraries(j["conversions"], conversion_libraries, extension, true);

    return true;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
