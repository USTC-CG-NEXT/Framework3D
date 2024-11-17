//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_NUMLIGHTSNODEGLSL_H
#define MATERIALX_NUMLIGHTSNODEGLSL_H

#include "../GlslShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Utility node for getting number of active lights for GLSL.
class HD_USTC_CG_API NumLightsNodeGlsl : public GlslImplementation
{
  public:
    NumLightsNodeGlsl();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
