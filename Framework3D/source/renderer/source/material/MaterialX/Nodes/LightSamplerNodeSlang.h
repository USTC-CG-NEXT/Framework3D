//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTSAMPLERNODESLANG_H
#define MATERIALX_LIGHTSAMPLERNODESLANG_H

#include "../SlangShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Utility node for sampling lights for SLANG.
class HD_USTC_CG_API LightSamplerNodeSlang : public SlangImplementation
{
  public:
    LightSamplerNodeSlang();

    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
