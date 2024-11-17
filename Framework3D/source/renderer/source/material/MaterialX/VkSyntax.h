//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKSYNTAX_H
#define MATERIALX_VKSYNTAX_H

/// @file
/// Vulkan SLANG syntax class

#include "SlangSyntax.h"

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for Vulkan SLANG
class HD_USTC_CG_API VkSyntax : public SlangSyntax
{
  public:
    VkSyntax();

    static SyntaxPtr create() { return std::make_shared<VkSyntax>(); }

    const string& getInputQualifier() const override { return INPUT_QUALIFIER; }
};

MATERIALX_NAMESPACE_END

#endif
