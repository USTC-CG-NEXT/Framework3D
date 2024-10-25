#pragma once

#include "node_socket.hpp"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace decl {

// Here we don't use a template style since there are not many types following
// this style... Later we may have all kinds of images and shaders.

class IntBuilder;

class Int : public SocketDeclaration {
   public:
    Int()
    {
        type = SocketType::Int;
    }

    NodeSocket* build(NodeTree* ntree, Node* node) const override;

    void update_default_value(NodeSocket* socket) const override;

    using Builder = IntBuilder;
    // TODO: Throw error on mac.
    // using DefaultValueType = bNodeSocketValue;

    int soft_min = -10;
    int soft_max = 10;
    int default_value_ = 0;
};

class IntBuilder : public SocketDeclarationBuilder<Int> {
   public:
    IntBuilder& min(int val)
    {
        decl_->soft_min = val;
        return *this;
    }

    IntBuilder& max(int val)
    {
        decl_->soft_max = val;
        return *this;
    }

    IntBuilder& default_val(int val)
    {
        decl_->default_value_ = val;
        return *this;
    }
};

class FloatBuilder;

class Float : public SocketDeclaration {
   public:
    Float()
    {
        type = SocketType::Float;
    }

    NodeSocket* build(NodeTree* ntree, Node* node) const override;

    void update_default_value(NodeSocket* socket) const override;

    using Builder = FloatBuilder;

    float soft_min = 0.f;
    float soft_max = 1.f;
    float default_value_ = 0;
};

class FloatBuilder : public SocketDeclarationBuilder<Float> {
   public:
    FloatBuilder& min(float val)
    {
        decl_->soft_min = val;
        return *this;
    }

    FloatBuilder& max(float val)
    {
        decl_->soft_max = val;
        return *this;
    }

    FloatBuilder& default_val(float val)
    {
        decl_->default_value_ = val;
        return *this;
    }
};

class StringBuilder;

class String : public SocketDeclaration {
   public:
    String()
    {
        type = SocketType::String;
    }

    NodeSocket* build(NodeTree* ntree, Node* node) const override;

    void update_default_value(NodeSocket* socket) const override;

    using Builder = StringBuilder;

    std::string default_value_ = {};
};

class StringBuilder : public SocketDeclarationBuilder<String> {
   public:
    StringBuilder& default_val(const std::string& val)
    {
        decl_->default_value_ = val;
        return *this;
    }
};

class AnyBuilder;

class Any : public SocketDeclaration {
   public:
    Any()
    {
        type = SocketType::Any;
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = AnyBuilder;
};

class AnyBuilder : public SocketDeclarationBuilder<Any> { };

class BoolBuilder;

class Bool : public SocketDeclaration {
   public:
    Bool()
    {
        type = SocketType::Bool;
    }

    NodeSocket* build(NodeTree* ntree, Node* node) const override;

    void update_default_value(NodeSocket* socket) const override;

    using Builder = BoolBuilder;

    bool soft_min = 0;
    bool soft_max = 1;
    bool default_value_ = 0;
};

class BoolBuilder : public SocketDeclarationBuilder<Bool> {
   public:
    BoolBuilder& min(bool val)
    {
        decl_->soft_min = val;
        return *this;
    }

    BoolBuilder& max(bool val)
    {
        decl_->soft_max = val;
        return *this;
    }

    BoolBuilder& default_val(bool val)
    {
        decl_->default_value_ = val;
        return *this;
    }
};

}  // namespace decl

USTC_CG_NAMESPACE_CLOSE_SCOPE