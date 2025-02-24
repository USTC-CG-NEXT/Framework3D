//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "UnlitSurfaceNodeSlang.h"

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr UnlitSurfaceNodeSlang::create()
{
    return std::make_shared<UnlitSurfaceNodeSlang>();
}

void UnlitSurfaceNodeSlang::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const SlangShaderGenerator& shadergen = static_cast<const SlangShaderGenerator&>(context.getShaderGenerator());

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Declare the output variable
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, true, context, stage);
        shadergen.emitLineEnd(stage);

        const string outColor = output->getVariable() + ".color";
        const string outTransparency = output->getVariable() + ".transparency";

        const ShaderInput* emission = node.getInput("emission");
        const ShaderInput* emissionColor = node.getInput("emission_color");
        shadergen.emitLine(outColor + " = " + shadergen.getUpstreamResult(emission, context) + " * " + shadergen.getUpstreamResult(emissionColor, context), stage);

        const ShaderInput* transmission = node.getInput("transmission");
        const ShaderInput* transmissionColor = node.getInput("transmission_color");
        shadergen.emitLine(outTransparency + " = " + shadergen.getUpstreamResult(transmission, context) + " * " + shadergen.getUpstreamResult(transmissionColor, context), stage);

        const ShaderInput* opacity = node.getInput("opacity");
        const string surfaceOpacity = shadergen.getUpstreamResult(opacity, context);
        shadergen.emitLine(outColor + " *= " + surfaceOpacity, stage);
        shadergen.emitLine(outTransparency + " = lerp(float3(1.0), " + outTransparency + ", " + surfaceOpacity + ")", stage);
    }
}

MATERIALX_NAMESPACE_END
