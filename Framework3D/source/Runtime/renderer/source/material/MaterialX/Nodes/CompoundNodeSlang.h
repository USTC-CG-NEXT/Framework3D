//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_COMPOUNDNODE_SLANG_H
#define MATERIALX_COMPOUNDNODE_SLANG_H

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>

#include "../SlangShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Compound node implementation
class HD_USTC_CG_API CompoundNodeSlang : public ShaderNodeImpl {
   public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context)
        override;

    void setValues(
        const Node& node,
        ShaderNode& shaderNode,
        GenContext& context) const override;

    void createVariables(
        const ShaderNode& node,
        GenContext& context,
        Shader& shader) const override;

    void emitFunctionDefinition(
        const ShaderNode& node,
        GenContext& context,
        ShaderStage& stage) const override;

    void emitFunctionCall(
        const ShaderNode& node,
        GenContext& context,
        ShaderStage& stage) const override;

    ShaderGraph* getGraph() const override
    {
        return _rootGraph.get();
    }

   protected:
    mutable ShaderGraphPtr _rootGraph;
    mutable string _functionName;
};

MATERIALX_NAMESPACE_END

#endif
