#pragma once
#include <nodes/core/node_exec.hpp>
#include <string>

#define NODE_DEF_OPEN_SCOPE \
    extern "C" {            \
    namespace USTC_CG {

#define NODE_DEF_CLOSE_SCOPE \
    }                        \
    }

#define NODE_DECLARATION_FUNCTION(name)             \
    __declspec(dllexport) void node_declare_##name( \
        USTC_CG::NodeDeclarationBuilder& b)

#define NODE_EXECUTION_FUNCTION(name) \
    __declspec(dllexport) void node_execution_##name(ExeParams params)

#define NODE_DECLARATION_UI(name) \
    __declspec(dllexport) const char* node_ui_name_##name()

#define CONVERSION_DECLARATION_FUNCTION(from, to)             \
    __declspec(dllexport) void node_declare_##from##_to_##to( \
        USTC_CG::NodeDeclarationBuilder& b)

#define CONVERSION_EXECUTION_FUNCTION(from, to) \
    __declspec(dllexport) void node_execution_##from##_to_##to(ExeParams params)

#define CONVERSION_FUNC_NAME(from, to)                                \
    __declspec(dllexport) std::string node_id_name_##from##_to_##to() \
    {                                                                 \
        return "conv_" + std::string(typeid(from).name()) + "_to_" +  \
               std::string(typeid(to).name());                        \
    }

#define NODE_DECLARATION_REQUIRED(name)               \
    __declspec(dllexport) bool node_required_##name() \
    {                                                 \
        return true;                                  \
    }