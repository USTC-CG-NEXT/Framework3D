//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTSAMPLERNODEGLSL_H
#define MATERIALX_LIGHTSAMPLERNODEGLSL_H

#include "GlslShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Utility node for sampling lights for GLSL.
class HD_USTC_CG_API LightSamplerNodeGlsl : public GlslImplementation
{
  public:
    LightSamplerNodeGlsl();

    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
