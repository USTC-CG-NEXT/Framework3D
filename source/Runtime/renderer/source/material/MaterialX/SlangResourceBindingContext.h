//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGRESOURCEBINDING_H
#define MATERIALX_SLANGRESOURCEBINDING_H

/// @file
/// SLANG resource binding context

#include "Export.h"

#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a SlangResourceBindingContext
using SlangResourceBindingContextPtr = shared_ptr<class SlangResourceBindingContext>;

/// @class SlangResourceBindingContext
/// Class representing a resource binding for Slang shader resources.
class HD_USTC_CG_API SlangResourceBindingContext : public HwResourceBindingContext
{
  public:
    SlangResourceBindingContext(size_t uniformBindingLocation, size_t samplerBindingLocation);

    static SlangResourceBindingContextPtr create(
        size_t uniformBindingLocation = 0, size_t samplerBindingLocation = 0)
    {
        return std::make_shared<SlangResourceBindingContext>(
            uniformBindingLocation, samplerBindingLocation);
    }

    // Initialize the context before generation starts.
    void initialize() override;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) override;

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;

    // Emit structured uniforms with binding information and align members where possible
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix) override;

    // Emit separate binding locations for sampler and uniform table
    void enableSeparateBindingLocations(bool separateBindingLocation) { _separateBindingLocation = separateBindingLocation; };

  protected:
    // List of required extensions
    StringSet _requiredExtensions;

    // Binding location for uniform blocks
    size_t _hwUniformBindLocation = 0;

    // Initial value of uniform binding location
    size_t _hwInitUniformBindLocation = 0;

    // Binding location for sampler blocks
    size_t _hwSamplerBindLocation = 0;

    // Initial value of sampler binding location
    size_t _hwInitSamplerBindLocation = 0;

    // Separate binding locations flag
    // Indicates whether to use a shared binding counter for samplers and uniforms or separate ones.
    // By default a shader counter is used.
    bool _separateBindingLocation = false;
};

MATERIALX_NAMESPACE_END

#endif
