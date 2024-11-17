//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GEOMPROPVALUENODEGLSL_H
#define MATERIALX_GEOMPROPVALUENODEGLSL_H

#include "../GlslShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// GeomPropValue node implementation for GLSL
class HD_USTC_CG_API GeomPropValueNodeGlsl : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    bool isEditable(const ShaderInput& /*input*/) const override { return false; }
};

/// GeomPropValue node non-implementation for GLSL
class HD_USTC_CG_API GeomPropValueNodeGlslAsUniform : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
