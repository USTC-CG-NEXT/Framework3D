#pragma once
#include <memory>
#include <string>

#include "USTC_CG.h"
#include "node_declare.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct SocketType;

enum class NodeTypeOfGrpah {
    Geometry,
    Function,
    Render,
    Composition,
    Conversion
};

struct ExeParams;
class Operator;

using ExecFunction = std::function<void(ExeParams params)>;
using NodeDeclareFunction =
    std::function<void(NodeDeclarationBuilder& builder)>;

// There can be many instances of nodes, while each of them has a type. The
// templates should be declared statically. It contains the information of the
// type of input and output.
struct NodeTypeInfo {
    explicit NodeTypeInfo(const char* id_name)
        : id_name(id_name),
          ui_name(id_name)
    {
        static_declaration = std::make_unique<NodeDeclaration>();
    }

    std::string id_name;
    std::string ui_name;

    void set_declare_function(const NodeDeclareFunction& decl_function)
    {
        this->declare = decl_function;
        build_node_declaration();
    }

    NodeTypeOfGrpah node_type_of_grpah;

    float color[4];
    ExecFunction node_execute;

    bool ALWAYS_REQUIRED = false;
    bool INVISIBLE = false;

    std::unique_ptr<NodeDeclaration> static_declaration;

    SocketType* conversion_from;
    SocketType* conversion_to;

   private:
    NodeDeclareFunction declare;

    void reset_declaration()
    {
        static_declaration = nullptr;
        static_declaration = std::make_unique<NodeDeclaration>();
    }

    void build_node_declaration()
    {
        reset_declaration();
        NodeDeclarationBuilder node_decl_builder{ *static_declaration };
        declare(node_decl_builder);
    }
};

namespace node {
std ::unique_ptr<NodeTypeInfo> make_node_type_info();

}  // namespace node

USTC_CG_NAMESPACE_CLOSE_SCOPE
