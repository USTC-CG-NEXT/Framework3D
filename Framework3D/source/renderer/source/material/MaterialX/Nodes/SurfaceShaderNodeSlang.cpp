//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "SurfaceShaderNodeSlang.h"
#include "../SlangShaderGenerator.h"

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SurfaceShaderNodeSlang::create()
{
    return std::make_shared<SurfaceShaderNodeSlang>();
}

const string& SurfaceShaderNodeSlang::getTarget() const
{
    return SlangShaderGenerator::TARGET;
}

void SurfaceShaderNodeSlang::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // TODO:
    // The surface shader needs position, view position and light sources. We should solve this by adding some
    // dependency mechanism so this implementation can be set to depend on the HwPositionNode,
    // HwViewDirectionNode and LightNodeSlang nodes instead? This is where the MaterialX attribute "internalgeomprops"
    // is needed.
    //
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, HW::T_VIEW_POSITION, ps);

    const SlangShaderGenerator& shadergen = static_cast<const SlangShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void SurfaceShaderNodeSlang::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const SlangShaderGenerator& shadergen = static_cast<const SlangShaderGenerator&>(context.getShaderGenerator());

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            context.getShaderGenerator().emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        SourceCodeNode::emitFunctionCall(node, context, stage);
    }
}

MATERIALX_NAMESPACE_END
