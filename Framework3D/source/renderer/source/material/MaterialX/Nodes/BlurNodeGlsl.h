//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_BLURNODEGLSL_H
#define MATERIALX_BLURNODEGLSL_H

#include "../Export.h"

#include <MaterialXGenShader/Nodes/BlurNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Blur node implementation for GLSL
class HD_USTC_CG_API BlurNodeGlsl : public BlurNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitSamplingFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
