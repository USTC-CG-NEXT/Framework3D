#pragma once
#include <nodes/core/node_exec.hpp>

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

#define NODE_DECLARATION_REQUIRED(name)               \
    __declspec(dllexport) bool node_required_##name() \
    {                                                 \
        return true;                                  \
    }