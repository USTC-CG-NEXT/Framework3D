//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTSHADERNODESLANG_H
#define MATERIALX_LIGHTSHADERNODESLANG_H

#include "../SlangShaderGenerator.h"
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// LightShader node implementation for SLANG
/// Used for all light shaders implemented in source code.
class HD_USTC_CG_API LightShaderNodeSlang : public SourceCodeNode
{
  public:
    LightShaderNodeSlang();

    static ShaderNodeImplPtr create();

    const string& getTarget() const override;

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END

#endif
