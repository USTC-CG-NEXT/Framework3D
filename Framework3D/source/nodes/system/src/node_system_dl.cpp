#include "node_system_dl.hpp"

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
    // Your implementation here
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

USTC_CG_NAMESPACE_CLOSE_SCOPE
