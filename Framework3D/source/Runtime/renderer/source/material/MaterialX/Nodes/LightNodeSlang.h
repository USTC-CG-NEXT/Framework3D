//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTNODESLANG_H
#define MATERIALX_LIGHTNODESLANG_H

#include "../SlangShaderGenerator.h"

MATERIALX_NAMESPACE_BEGIN

/// Light node implementation for SLANG
class HD_USTC_CG_API LightNodeSlang : public SlangImplementation
{
  public:
    LightNodeSlang();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  private:
    mutable ClosureContext _callEmission;
};

MATERIALX_NAMESPACE_END

#endif
