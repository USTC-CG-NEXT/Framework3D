//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GEOMPROPVALUENODESLANG_H
#define MATERIALX_GEOMPROPVALUENODESLANG_H

#include "../SlangShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// GeomPropValue node implementation for SLANG
class HD_USTC_CG_API GeomPropValueNodeSlang : public SlangImplementation
{
  public:
    static ShaderNodeImplPtr create();



    void setValues(
        const Node& node,
        ShaderNode& shaderNode,
        GenContext& context) const override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    bool isEditable(const ShaderInput& /*input*/) const override { return false; }

    std::string get_geomname(const ShaderNode& node) const;
};

/// GeomPropValue node non-implementation for SLANG
class HD_USTC_CG_API GeomPropValueNodeSlangAsUniform : public SlangImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
