//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "GeomPropValueNodeSlang.h"

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr GeomPropValueNodeSlang::create()
{
    return std::make_shared<GeomPropValueNodeSlang>();
}

void GeomPropValueNodeSlang::setValues(
    const Node& node,
    ShaderNode& shaderNode,
    GenContext& context) const
{
    SlangImplementation::setValues(node, shaderNode, context);
}

void GeomPropValueNodeSlang::createVariables(
    const ShaderNode& node,
    GenContext&,
    Shader& shader) const
{
    string geomProp = get_geomname(node);

    const ShaderOutput* output = node.getOutput();

    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(
        HW::VERTEX_INPUTS,
        output->getType(),
        HW::T_IN_GEOMPROP + "_" + geomProp,
        vs);
    addStageConnector(
        HW::VERTEX_DATA,
        output->getType(),
        HW::T_IN_GEOMPROP + "_" + geomProp,
        vs,
        ps);
}

void GeomPropValueNodeSlang::emitFunctionCall(
    const ShaderNode& node,
    GenContext& context,
    ShaderStage& stage) const
{
    const SlangShaderGenerator& shadergen =
        static_cast<const SlangShaderGenerator&>(context.getShaderGenerator());

    const string geomname = get_geomname(node);
    const string variable = HW::T_IN_GEOMPROP + "_" + geomname;

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* geomprop = vertexData[variable];
        if (!geomprop->isEmitted()) {
            shadergen.emitLine(
                prefix + geomprop->getVariable() + " = " + HW::T_IN_GEOMPROP +
                    "_" + geomname,
                stage);
            geomprop->setEmitted();
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* geomprop = vertexData[variable];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + prefix + geomprop->getVariable(), stage);
        shadergen.emitLineEnd(stage);
    }
}

std::string GeomPropValueNodeSlang::get_geomname(const ShaderNode& node) const
{
    const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
    auto connectedOutput = geomPropInput->getConnection();

    if ((!geomPropInput || !geomPropInput->getValue()) &&
        (!connectedOutput || !connectedOutput->getValue())) {
        throw ExceptionShaderGenError(
            "No 'geomprop' parameter found on geompropvalue node '" +
            node.getName() + "'. Don't know what property to bind");
    }

    std::string geomname;

    if (geomPropInput->getValue())
        geomname = geomPropInput->getValue()->getValueString();
    else
        geomname = connectedOutput->getValueString();

    return geomname;
}

ShaderNodeImplPtr GeomPropValueNodeSlangAsUniform::create()
{
    return std::make_shared<GeomPropValueNodeSlangAsUniform>();
}

void GeomPropValueNodeSlangAsUniform::createVariables(
    const ShaderNode& node,
    GenContext&,
    Shader& shader) const
{
    const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
    if (!geomPropInput || !geomPropInput->getValue()) {
        throw ExceptionShaderGenError(
            "No 'geomprop' parameter found on geompropvalue node '" +
            node.getName() + "'. Don't know what property to bind");
    }
    const string geomProp = geomPropInput->getValue()->getValueString();
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    ShaderPort* uniform = addStageUniform(
        HW::PRIVATE_UNIFORMS,
        node.getOutput()->getType(),
        HW::T_GEOMPROP + "_" + geomProp,
        ps);
    uniform->setPath(geomPropInput->getPath());
}

void GeomPropValueNodeSlangAsUniform::emitFunctionCall(
    const ShaderNode& node,
    GenContext& context,
    ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
        if (!geomPropInput) {
            throw ExceptionShaderGenError(
                "No 'geomprop' parameter found on geompropvalue node '" +
                node.getName() + "'. Don't know what property to bind");
        }
        const string attrName = geomPropInput->getValue()->getValueString();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + HW::T_GEOMPROP + "_" + attrName, stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
