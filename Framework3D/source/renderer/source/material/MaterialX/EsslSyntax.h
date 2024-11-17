//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_ESSLSYNTAX_H
#define MATERIALX_ESSLSYNTAX_H

/// @file
/// ESSL syntax class

#include "SlangSyntax.h"

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for ESSL (OpenGL ES Shading Language)
class HD_USTC_CG_API EsslSyntax : public SlangSyntax
{
  public:
    EsslSyntax();

    static SyntaxPtr create() { return std::make_shared<EsslSyntax>(); }
};

MATERIALX_NAMESPACE_END

#endif
