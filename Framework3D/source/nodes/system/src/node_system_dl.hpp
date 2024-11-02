#pragma once

#include <nodes/system/api.h>

#include <stdexcept>
#include <string>

#include "nodes/system/node_system.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

USTC_CG_NAMESPACE_OPEN_SCOPE

class NODES_SYSTEM_API DynamicLibraryLoader {
   public:
    DynamicLibraryLoader(const std::string& libraryName);

    ~DynamicLibraryLoader();

    template<typename Func>
    Func getFunction(const std::string& functionName);

   private:
#ifdef _WIN32
    HMODULE handle;
#else
    void* handle;
#endif
};

template<typename Func>
Func DynamicLibraryLoader::getFunction(const std::string& functionName)
{
#ifdef _WIN32
    FARPROC funcPtr = GetProcAddress(handle, functionName.c_str());
    if (!funcPtr) {
        throw std::runtime_error("Failed to load function: " + functionName);
    }
    return reinterpret_cast<Func>(funcPtr);
#else
    void* funcPtr = dlsym(handle, functionName.c_str());
    if (!funcPtr) {
        throw std::runtime_error("Failed to load function: " + functionName);
    }
    return reinterpret_cast<Func>(funcPtr);
#endif
}

class NODES_SYSTEM_API NodeDynamicLoadingSystem : public NodeSystem {
    NodeTreeDescriptor node_tree_descriptor() override;
    NodeTreeExecutorDesc node_tree_executor_desc() override;

   public:
    ~NodeDynamicLoadingSystem() override;

   private:
    std::vector<std::unique_ptr<DynamicLibraryLoader>> libraries;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE