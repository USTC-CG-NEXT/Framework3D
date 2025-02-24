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
    std::function<Func> getFunction(const std::string& functionName);

   private:
#ifdef _WIN32
    HMODULE handle;
#else
    void* handle;
#endif
};

template<typename Func>
std::function<Func> DynamicLibraryLoader::getFunction(
    const std::string& functionName)
{
#ifdef _WIN32
    FARPROC funcPtr = GetProcAddress(handle, functionName.c_str());
    if (!funcPtr) {
        return nullptr;
    }
    return reinterpret_cast<Func*>(funcPtr);
#else
    void* funcPtr = dlsym(handle, functionName.c_str());
    if (!funcPtr) {
        return nullptr;
    }
    return reinterpret_cast<Func*>(funcPtr);
#endif
}

class NODES_SYSTEM_API NodeDynamicLoadingSystem : public NodeSystem {
   protected:



    std::shared_ptr<NodeTreeDescriptor> node_tree_descriptor() override;

   public:

    NodeDynamicLoadingSystem();
    ~NodeDynamicLoadingSystem() override;
    bool load_configuration(const std::filesystem::path& config) override;

   private:
    std::unordered_map<std::string, std::unique_ptr<DynamicLibraryLoader>>
        node_libraries;
    std::unordered_map<std::string, std::unique_ptr<DynamicLibraryLoader>>
        conversion_libraries;
    std::shared_ptr<NodeTreeDescriptor> descriptor;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE