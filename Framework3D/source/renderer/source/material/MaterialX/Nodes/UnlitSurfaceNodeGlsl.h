//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_UNLITSURFACENODEGLSL_H
#define MATERIALX_UNLITSURFACENODEGLSL_H

#include "../Export.h"
#include "../GlslShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Unlit surface node implementation for GLSL
class HD_USTC_CG_API UnlitSurfaceNodeGlsl : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
