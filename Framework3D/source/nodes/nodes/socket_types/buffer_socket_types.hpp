#pragma once

#include "Macro/map.h"
#include "USTC_CG.h"
#include "all_socket_types.hpp"
#include "make_standard_type.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE
namespace decl {

class Int1BufferBuilder;
class Int1Buffer : public SocketDeclaration {
   public:
    Int1Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int1BufferBuilder;
};
class Int1BufferBuilder : public SocketDeclarationBuilder<Int1Buffer> { };
class Int2BufferBuilder;
class Int2Buffer : public SocketDeclaration {
   public:
    Int2Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int2BufferBuilder;
};
class Int2BufferBuilder : public SocketDeclarationBuilder<Int2Buffer> { };
class Int3BufferBuilder;
class Int3Buffer : public SocketDeclaration {
   public:
    Int3Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int3BufferBuilder;
};
class Int3BufferBuilder : public SocketDeclarationBuilder<Int3Buffer> { };
class Int4BufferBuilder;
class Int4Buffer : public SocketDeclaration {
   public:
    Int4Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int4BufferBuilder;
};
class Int4BufferBuilder : public SocketDeclarationBuilder<Int4Buffer> { };
class Float1BufferBuilder;
class Float1Buffer : public SocketDeclaration {
   public:
    Float1Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float1BufferBuilder;
};
class Float1BufferBuilder : public SocketDeclarationBuilder<Float1Buffer> { };
class Float2BufferBuilder;
class Float2Buffer : public SocketDeclaration {
   public:
    Float2Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float2BufferBuilder;
};
class Float2BufferBuilder : public SocketDeclarationBuilder<Float2Buffer> { };
class Float3BufferBuilder;
class Float3Buffer : public SocketDeclaration {
   public:
    Float3Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float3BufferBuilder;
};
class Float3BufferBuilder : public SocketDeclarationBuilder<Float3Buffer> { };
class Float4BufferBuilder;
class Float4Buffer : public SocketDeclaration {
   public:
    Float4Buffer()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float4BufferBuilder;
};
class Float4BufferBuilder : public SocketDeclarationBuilder<Float4Buffer> { };
class Int2Builder;
class Int2 : public SocketDeclaration {
   public:
    Int2()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int2Builder;
};
class Int2Builder : public SocketDeclarationBuilder<Int2> { };
class Int3Builder;
class Int3 : public SocketDeclaration {
   public:
    Int3()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int3Builder;
};
class Int3Builder : public SocketDeclarationBuilder<Int3> { };
class Int4Builder;
class Int4 : public SocketDeclaration {
   public:
    Int4()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Int4Builder;
};
class Int4Builder : public SocketDeclarationBuilder<Int4> { };
class Float2Builder;
class Float2 : public SocketDeclaration {
   public:
    Float2()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float2Builder;
};
class Float2Builder : public SocketDeclarationBuilder<Float2> { };
class Float3Builder;
class Float3 : public SocketDeclaration {
   public:
    Float3()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float3Builder;
};
class Float3Builder : public SocketDeclarationBuilder<Float3> { };
class Float4Builder;
class Float4 : public SocketDeclaration {
   public:
    Float4()
    {
    }
    NodeSocket* build(NodeTree* ntree, Node* node) const override;
    using Builder = Float4Builder;
};
class Float4Builder : public SocketDeclarationBuilder<Float4> { };

}  // namespace decl

USTC_CG_NAMESPACE_CLOSE_SCOPE