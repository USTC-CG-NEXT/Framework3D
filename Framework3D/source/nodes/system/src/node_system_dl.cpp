#include "node_system_dl.hpp"

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
        FreeLibrary(handle);
    }
#else
    if (handle) {
        dlclose(handle);
    }
#endif
}

NodeTreeDescriptor NodeDynamicLoadingSystem::node_tree_descriptor()
{
    register_cpp_type<int>();

    // register adding node

    for (auto&& library : node_libraries) {
        auto node_ui_name =
            library.second->getFunction<const char*()>("node_ui_name");

        auto node_declare =
            library.second->getFunction<void(NodeDeclarationBuilder&)>(
                "node_declare");
        auto node_execution =
            library.second->getFunction<void(ExeParams)>("node_execution");

        NodeTypeInfo new_node;
        new_node.id_name = library.first;
        new_node.ui_name = node_ui_name ? node_ui_name() : library.first;
        new_node.set_declare_function(node_declare);
        new_node.set_execution_function(node_execution);

        descriptor.register_node(new_node);
    }

    return {};
}

NodeTreeExecutorDesc NodeDynamicLoadingSystem::node_tree_executor_desc()
{
    // Your implementation here
    return {};
}

NodeDynamicLoadingSystem::~NodeDynamicLoadingSystem()
{
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

    // Process the JSON configuration as needed

    auto load_libraries = [&](const nlohmann::json& json_section,
                              auto& library_map,
                              const std::string& extension) {
        for (auto it = json_section.begin(); it != json_section.end(); ++it) {
            std::string key = it.key();
            std::string libNameStr = it.value().get<std::string>();
            library_map[key] =
                std::make_unique<DynamicLibraryLoader>(libNameStr + extension);
        }
    };

#ifdef _WIN32
    std::string extension = ".dll";
#else
    std::string extension = ".so";
#endif

    load_libraries(j["nodes"], node_libraries, extension);
    load_libraries(j["conversions"], conversion_libraries, extension);

    return true;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
