//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKSHADERGENERATOR_H
#define MATERIALX_VKSHADERGENERATOR_H

/// @file
/// Vulkan SLANG shader generator

#include "SlangShaderGenerator.h"
#include "VkResourceBindingContext.h"

MATERIALX_NAMESPACE_BEGIN

using VkShaderGeneratorPtr = shared_ptr<class VkShaderGenerator>;

/// @class VkShaderGenerator
/// A Vulkan SLANG shader generator
class HD_USTC_CG_API VkShaderGenerator : public SlangShaderGenerator
{
  public:
    VkShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<VkShaderGenerator>(); }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the SLANG version this generator is for
    const string& getVersion() const override { return VERSION; }

    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

    /// Unique identifier for this generator target
    static const string TARGET;
    static const string VERSION;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) const override;

    void emitInputs(GenContext& context, ShaderStage& stage) const override;

    void emitOutputs(GenContext& context, ShaderStage& stage) const override;

  protected:
    HwResourceBindingContextPtr getResourceBindingContext(GenContext&) const override;

    VkResourceBindingContextPtr _resourceBindingCtx = nullptr;
};

MATERIALX_NAMESPACE_END

#endif
