//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GEOMCOLORNODEGLSL_H
#define MATERIALX_GEOMCOLORNODEGLSL_H

#include "GlslShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// GeomColor node implementation for GLSL
class HD_USTC_CG_API GeomColorNodeGlsl : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
