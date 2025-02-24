//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_UNLITSURFACENODESLANG_H
#define MATERIALX_UNLITSURFACENODESLANG_H

#include "../Export.h"
#include "../SlangShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Unlit surface node implementation for SLANG
class HD_USTC_CG_API UnlitSurfaceNodeSlang : public SlangImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
