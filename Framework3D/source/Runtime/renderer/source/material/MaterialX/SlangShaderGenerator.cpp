//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "SlangShaderGenerator.h"

#include <MaterialXGenShader/Nodes/ClosureAddNode.h>
#include <MaterialXGenShader/Nodes/ClosureCompoundNode.h>
#include <MaterialXGenShader/Nodes/ClosureLayerNode.h>
#include <MaterialXGenShader/Nodes/ClosureMixNode.h>
#include <MaterialXGenShader/Nodes/ClosureMultiplyNode.h>
#include <MaterialXGenShader/Nodes/ClosureSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/HwFrameNode.h>
#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/Nodes/HwPositionNode.h>
#include <MaterialXGenShader/Nodes/HwTexCoordNode.h>
#include <MaterialXGenShader/Nodes/HwTimeNode.h>
#include <MaterialXGenShader/Nodes/HwTransformNode.h>
#include <MaterialXGenShader/Nodes/HwViewDirectionNode.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>

#include <format>

#include "Logger/Logger.h"
#include "Nodes/BitangentNodeSlang.h"
#include "Nodes/BlurNodeSlang.h"
#include "Nodes/CompoundNodeSlang.h"
#include "Nodes/GeomColorNodeSlang.h"
#include "Nodes/GeomPropValueNodeSlang.h"
#include "Nodes/HeightToNormalNodeSlang.h"
#include "Nodes/LightCompoundNodeSlang.h"
#include "Nodes/LightNodeSlang.h"
#include "Nodes/LightSamplerNodeSlang.h"
#include "Nodes/LightShaderNodeSlang.h"
#include "Nodes/NormalNodeSlang.h"
#include "Nodes/NumLightsNodeSlang.h"
#include "Nodes/SurfaceNodeSlang.h"
#include "Nodes/TangentNodeSlang.h"
#include "Nodes/UnlitSurfaceNodeSlang.h"
#include "SlangSyntax.h"

MATERIALX_NAMESPACE_BEGIN
const string SlangShaderGenerator::TARGET = "genslang";
const string SlangShaderGenerator::VERSION = "400";

//
// SlangShaderGenerator methods
//

SlangShaderGenerator::SlangShaderGenerator()
    : HwShaderGenerator(SlangSyntax::create())
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <switch> -->
    elementNames = {
        // <!-- 'which' type : float -->
        "IM_switch_float_" + SlangShaderGenerator::TARGET,
        "IM_switch_color3_" + SlangShaderGenerator::TARGET,
        "IM_switch_color4_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector2_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector3_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- 'which' type : integer -->
        "IM_switch_floatI_" + SlangShaderGenerator::TARGET,
        "IM_switch_color3I_" + SlangShaderGenerator::TARGET,
        "IM_switch_color4I_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector2I_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector3I_" + SlangShaderGenerator::TARGET,
        "IM_switch_vector4I_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, SwitchNode::create);

    // <!-- <swizzle> -->
    elementNames = {
        // <!-- from type : float -->
        "IM_swizzle_float_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_float_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_float_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_float_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_float_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- from type : color3 -->
        "IM_swizzle_color3_float_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color3_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color3_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color3_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color3_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color3_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- from type : color4 -->
        "IM_swizzle_color4_float_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color4_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color4_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color4_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color4_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_color4_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- from type : vector2 -->
        "IM_swizzle_vector2_float_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector2_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector2_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector2_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector2_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector2_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- from type : vector3 -->
        "IM_swizzle_vector3_float_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector3_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector3_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector3_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector3_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector3_vector4_" + SlangShaderGenerator::TARGET,

        // <!-- from type : vector4 -->
        "IM_swizzle_vector4_float_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector4_color3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector4_color4_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector4_vector2_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector4_vector3_" + SlangShaderGenerator::TARGET,
        "IM_swizzle_vector4_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, SwizzleNode::create);

    // <!-- <convert> -->
    elementNames = {
        "IM_convert_float_color3_" + SlangShaderGenerator::TARGET,
        "IM_convert_float_color4_" + SlangShaderGenerator::TARGET,
        "IM_convert_float_vector2_" + SlangShaderGenerator::TARGET,
        "IM_convert_float_vector3_" + SlangShaderGenerator::TARGET,
        "IM_convert_float_vector4_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector2_vector3_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector3_vector2_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector3_color3_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector3_vector4_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector4_vector3_" + SlangShaderGenerator::TARGET,
        "IM_convert_vector4_color4_" + SlangShaderGenerator::TARGET,
        "IM_convert_color3_vector3_" + SlangShaderGenerator::TARGET,
        "IM_convert_color4_vector4_" + SlangShaderGenerator::TARGET,
        "IM_convert_color3_color4_" + SlangShaderGenerator::TARGET,
        "IM_convert_color4_color3_" + SlangShaderGenerator::TARGET,
        "IM_convert_boolean_float_" + SlangShaderGenerator::TARGET,
        "IM_convert_integer_float_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, ConvertNode::create);

    // <!-- <combine> -->
    elementNames = {
        "IM_combine2_vector2_" + SlangShaderGenerator::TARGET,
        "IM_combine2_color4CF_" + SlangShaderGenerator::TARGET,
        "IM_combine2_vector4VF_" + SlangShaderGenerator::TARGET,
        "IM_combine2_vector4VV_" + SlangShaderGenerator::TARGET,
        "IM_combine3_color3_" + SlangShaderGenerator::TARGET,
        "IM_combine3_vector3_" + SlangShaderGenerator::TARGET,
        "IM_combine4_color4_" + SlangShaderGenerator::TARGET,
        "IM_combine4_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, CombineNode::create);

    // <!-- <position> -->
    registerImplementation(
        "IM_position_vector3_" + SlangShaderGenerator::TARGET,
        HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation(
        "IM_normal_vector3_" + SlangShaderGenerator::TARGET,
        NormalNodeSlang::create);
    // <!-- <tangent> -->
    registerImplementation(
        "IM_tangent_vector3_" + SlangShaderGenerator::TARGET,
        TangentNodeSlang::create);
    // <!-- <bitangent> -->
    registerImplementation(
        "IM_bitangent_vector3_" + SlangShaderGenerator::TARGET,
        BitangentNodeSlang::create);
    // <!-- <texcoord> -->
    registerImplementation(
        "IM_texcoord_vector2_" + SlangShaderGenerator::TARGET,
        HwTexCoordNode::create);
    registerImplementation(
        "IM_texcoord_vector3_" + SlangShaderGenerator::TARGET,
        HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation(
        "IM_geomcolor_float_" + SlangShaderGenerator::TARGET,
        GeomColorNodeSlang::create);
    registerImplementation(
        "IM_geomcolor_color3_" + SlangShaderGenerator::TARGET,
        GeomColorNodeSlang::create);
    registerImplementation(
        "IM_geomcolor_color4_" + SlangShaderGenerator::TARGET,
        GeomColorNodeSlang::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, GeomPropValueNodeSlang::create);
    registerImplementation(
        "IM_geompropvalue_boolean_" + SlangShaderGenerator::TARGET,
        GeomPropValueNodeSlangAsUniform::create);
    registerImplementation(
        "IM_geompropvalue_string_" + SlangShaderGenerator::TARGET,
        GeomPropValueNodeSlangAsUniform::create);

    // <!-- <frame> -->
    registerImplementation(
        "IM_frame_float_" + SlangShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation(
        "IM_time_float_" + SlangShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation(
        "IM_viewdirection_vector3_" + SlangShaderGenerator::TARGET,
        HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation(
        "IM_surface_" + SlangShaderGenerator::TARGET, SurfaceNodeSlang::create);
    registerImplementation(
        "IM_surface_unlit_" + SlangShaderGenerator::TARGET,
        UnlitSurfaceNodeSlang::create);

    // <!-- <light> -->
    registerImplementation(
        "IM_light_" + SlangShaderGenerator::TARGET, LightNodeSlang::create);

    // <!-- <point_light> -->
    registerImplementation(
        "IM_point_light_" + SlangShaderGenerator::TARGET,
        LightShaderNodeSlang::create);
    // <!-- <directional_light> -->
    registerImplementation(
        "IM_directional_light_" + SlangShaderGenerator::TARGET,
        LightShaderNodeSlang::create);
    // <!-- <spot_light> -->
    registerImplementation(
        "IM_spot_light_" + SlangShaderGenerator::TARGET,
        LightShaderNodeSlang::create);

    // <!-- <heighttonormal> -->
    registerImplementation(
        "IM_heighttonormal_vector3_" + SlangShaderGenerator::TARGET,
        HeightToNormalNodeSlang::create);

    // <!-- <blur> -->
    elementNames = {
        "IM_blur_float_" + SlangShaderGenerator::TARGET,
        "IM_blur_color3_" + SlangShaderGenerator::TARGET,
        "IM_blur_color4_" + SlangShaderGenerator::TARGET,
        "IM_blur_vector2_" + SlangShaderGenerator::TARGET,
        "IM_blur_vector3_" + SlangShaderGenerator::TARGET,
        "IM_blur_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, BlurNodeSlang::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation(
        "IM_transformpoint_vector3_" + SlangShaderGenerator::TARGET,
        HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation(
        "IM_transformvector_vector3_" + SlangShaderGenerator::TARGET,
        HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation(
        "IM_transformnormal_vector3_" + SlangShaderGenerator::TARGET,
        HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + SlangShaderGenerator::TARGET,
        "IM_image_color3_" + SlangShaderGenerator::TARGET,
        "IM_image_color4_" + SlangShaderGenerator::TARGET,
        "IM_image_vector2_" + SlangShaderGenerator::TARGET,
        "IM_image_vector3_" + SlangShaderGenerator::TARGET,
        "IM_image_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <layer> -->
    registerImplementation(
        "IM_layer_bsdf_" + SlangShaderGenerator::TARGET,
        ClosureLayerNode::create);
    registerImplementation(
        "IM_layer_vdf_" + SlangShaderGenerator::TARGET,
        ClosureLayerNode::create);
    // <!-- <lerp> -->
    registerImplementation(
        "IM_mix_bsdf_" + SlangShaderGenerator::TARGET, ClosureMixNode::create);
    registerImplementation(
        "IM_mix_edf_" + SlangShaderGenerator::TARGET, ClosureMixNode::create);
    // <!-- <add> -->
    registerImplementation(
        "IM_add_bsdf_" + SlangShaderGenerator::TARGET, ClosureAddNode::create);
    registerImplementation(
        "IM_add_edf_" + SlangShaderGenerator::TARGET, ClosureAddNode::create);
    // <!-- <multiply> -->
    elementNames = {
        "IM_multiply_bsdfC_" + SlangShaderGenerator::TARGET,
        "IM_multiply_bsdfF_" + SlangShaderGenerator::TARGET,
        "IM_multiply_edfC_" + SlangShaderGenerator::TARGET,
        "IM_multiply_edfF_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, ClosureMultiplyNode::create);

    // <!-- <thin_film> -->
    registerImplementation(
        "IM_thin_film_bsdf_" + SlangShaderGenerator::TARGET, NopNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation(
        "IM_surfacematerial_" + SlangShaderGenerator::TARGET,
        MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(
        nullptr, "numActiveLightSources", NumLightsNodeSlang::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(
        nullptr, "sampleLightSource", LightSamplerNodeSlang::create()));
}

ShaderPtr SlangShaderGenerator::generate(
    const string& name,
    ElementPtr element,
    GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx =
        getResourceBindingContext(context);
    if (resourceBindingCtx) {
        resourceBindingCtx->initialize();
    }

    // Emit code for vertex shader stage
    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

void SlangShaderGenerator::emitVertexStage(
    const ShaderGraph& graph,
    GenContext& context,
    ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx =
        getResourceBindingContext(context);

    emitDirectives(context, stage);
    if (resourceBindingCtx) {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    emitFunctionDefinitions(graph, context, stage);
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);

    // Add main function
    setFunctionName("main", stage);
    emitLine(
        "void main(out float4 sv_pos : SV_POSITION, out " +
            vertexData.getName() + " " + vertexData.getInstance() + ")",
        stage,
        false);
    emitFunctionBodyBegin(graph, context, stage);
    emitLine(
        "float4 hPositionWorld = mul(" + HW::T_WORLD_MATRIX + ", float4(" +
            HW::T_IN_POSITION + ", 1.0))",
        stage);
    emitLine(
        "sv_pos = mul(" + HW::T_VIEW_PROJECTION_MATRIX + ", hPositionWorld)",
        stage);

    // Emit all function calls in order
    for (const ShaderNode* node : graph.getNodes()) {
        emitFunctionCall(*node, context, stage);
    }

    emitFunctionBodyEnd(graph, context, stage);
}

void SlangShaderGenerator::emitSpecularEnvironment(
    GenContext& context,
    ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_environment_fis.slang", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_environment_prefilter.slang",
            context,
            stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_environment_none.slang", context, stage);
    }
    else {
        throw ExceptionShaderGenError(
            "Invalid hardware specular environment method specified: '" +
            std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitTransmissionRender(
    GenContext& context,
    ShaderStage& stage) const
{
    int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_transmission_refract.slang",
            context,
            stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_transmission_opacity.slang",
            context,
            stage);
    }
    else {
        throw ExceptionShaderGenError(
            "Invalid transmission render specified: '" +
            std::to_string(transmissionMethod) + "'");
    }
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
}

void SlangShaderGenerator::emitConstants(
    GenContext& context,
    ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty()) {
        emitVariableDeclarations(
            constants,
            _syntax->getConstantQualifier(),
            Syntax::SEMICOLON,
            context,
            stage);
        emitLineBreak(stage);
    }
}

void SlangShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage)
    const
{
    for (const auto& it : stage.getUniformBlocks()) {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA) {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            HwResourceBindingContextPtr resourceBindingCtx =
                getResourceBindingContext(context);
            if (resourceBindingCtx) {
                resourceBindingCtx->emitResourceBindings(
                    context, uniforms, stage);
            }
            else {
                emitVariableDeclarations(
                    uniforms,
                    _syntax->getUniformQualifier(),
                    Syntax::SEMICOLON,
                    context,
                    stage);
                emitLineBreak(stage);
            }
        }
    }
}

void SlangShaderGenerator::emitLightData(
    GenContext& context,
    ShaderStage& stage) const
{
    const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
    const string structArraySuffix =
        "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
    const string structName = lightData.getInstance();
    HwResourceBindingContextPtr resourceBindingCtx =
        getResourceBindingContext(context);
    if (resourceBindingCtx) {
        resourceBindingCtx->emitStructuredResourceBindings(
            context, lightData, stage, structName, structArraySuffix);
    }
    else {
        emitLine("struct " + lightData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(
            lightData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
        emitLine(
            "uniform " + lightData.getName() + " " + structName +
                structArraySuffix,
            stage);
    }
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage)
    const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs =
            stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty()) {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            emitVariableDeclarations(
                vertexInputs,
                _syntax->getInputQualifier(),
                Syntax::SEMICOLON,
                context,
                stage,
                false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty()) {
            emitLine("struct " + vertexData.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(
                vertexData,
                EMPTY_STRING,
                Syntax::SEMICOLON,
                context,
                stage,
                false);
            emitScopeEnd(stage, false, false);
            emitString(Syntax::SEMICOLON, stage);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
    }
}

void SlangShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage)
    const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty()) {
            emitLine("struct " + vertexData.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(
                vertexData,
                EMPTY_STRING,
                Syntax::SEMICOLON,
                context,
                stage,
                false);
            emitScopeEnd(stage, false, false);
            emitString(Syntax::SEMICOLON, stage);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        // emitVariableDeclarations(
        //     outputs,
        //     _syntax->getOutputQualifier(),
        //     Syntax::SEMICOLON,
        //     context,
        //     stage,
        //     false);
        // emitLineBreak(stage);
    }
}

HwResourceBindingContextPtr SlangShaderGenerator::getResourceBindingContext(
    GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(
        HW::USER_DATA_BINDING_CONTEXT);
}

string SlangShaderGenerator::getVertexDataPrefix(
    const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

bool SlangShaderGenerator::requiresLighting(const ShaderGraph& graph) const
{
    const bool isBsdf =
        graph.hasClassification(ShaderNode::Classification::BSDF);
    const bool isLitSurfaceShader =
        graph.hasClassification(ShaderNode::Classification::SHADER) &&
        graph.hasClassification(ShaderNode::Classification::SURFACE) &&
        !graph.hasClassification(ShaderNode::Classification::UNLIT);
    return isBsdf || isLitSurfaceShader;
}

void SlangShaderGenerator::emitPixelStage(
    const ShaderGraph& graph,
    GenContext& context,
    ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx =
        getResourceBindingContext(context);

    // Add directives
    emitDirectives(context, stage);
    if (resourceBindingCtx) {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add the pixel shader output. This needs to be a float4 for rendering
    // and upstream connection will be converted to float4 if needed in
    // emitFinalOutput()
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genslang/lib/mx_math.slang", context, stage);
    emitLineBreak(stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);

    // Define directional albedo approach
    if (lighting || context.getOptions().hwWriteAlbedoTable ||
        context.getOptions().hwWriteEnvPrefilter) {
        emitLine(
            "#define DIRECTIONAL_ALBEDO_METHOD " +
                std::to_string(
                    int(context.getOptions().hwDirectionalAlbedoMethod)),
            stage,
            false);
        emitLineBreak(stage);
    }

    // Add lighting support
    if (lighting) {
        if (context.getOptions().hwMaxActiveLightSources > 0) {
            const unsigned int maxLights =
                std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine(
                "#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " +
                    std::to_string(maxLights),
                stage,
                false);
        }
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);

        if (context.getOptions().hwMaxActiveLightSources > 0) {
            emitLightData(context, stage);
        }
    }

    // Add shadowing support
    bool shadowing = (lighting && context.getOptions().hwShadowMap) ||
                     context.getOptions().hwWriteDepthMoments;
    if (shadowing) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_shadow.slang", context, stage);
    }

    // Emit directional albedo table code.
    if (context.getOptions().hwWriteAlbedoTable) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_generate_albedo_table.slang",
            context,
            stage);
        emitLineBreak(stage);
    }

    // Emit environment prefiltering code
    if (context.getOptions().hwWriteEnvPrefilter) {
        emitLibraryInclude(
            "pbrlib/genslang/lib/mx_generate_prefilter_env.slang",
            context,
            stage);
        emitLineBreak(stage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    if (context.getOptions().fileTextureVerticalFlip) {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] =
            "mx_transform_uv_vflip.slang";
    }
    else {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] =
            "mx_transform_uv.slang";
    }

    // Emit uv transform code globally if needed.
    if (context.getOptions().hwAmbientOcclusion) {
        emitLibraryInclude(
            "stdlib/genslang/lib/" +
                _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV],
            context,
            stage);
    }

    emitLightFunctionDefinitions(graph, context, stage);

    // Emit function definitions for all nodes in the graph.
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    setFunctionName("main", stage);

    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);

    emitLine("void main(", stage, false);

    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    emitVariableDeclarations(
        outputs,
        _syntax->getOutputQualifier(),
        Syntax::COMMA,
        context,
        stage,
        false);

    emitLine(
        "in " + vertexData.getName() + " " + vertexData.getInstance() + ")",
        stage,
        false);

    emitFunctionBodyBegin(graph, context, stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER)) {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching
        // to a surface shader, so just output black.
        emitLine(
            outputSocket->getVariable() + " = float4(0.0, 0.0, 0.0, 1.0)",
            stage);
    }
    else if (context.getOptions().hwWriteDepthMoments) {
        emitLine(
            outputSocket->getVariable() +
                " = float4(mx_compute_depth_moments(), 0.0, 1.0)",
            stage);
    }
    else if (context.getOptions().hwWriteAlbedoTable) {
        emitLine(
            outputSocket->getVariable() +
                " = float4(mx_generate_dir_albedo_table(), 1.0)",
            stage);
    }
    else if (context.getOptions().hwWriteEnvPrefilter) {
        emitLine(
            outputSocket->getVariable() +
                " = float4(mx_generate_prefilter_env(), 1.0)",
            stage);
    }
    else {
        // Add all function calls.
        //
        // Surface shaders need special handling.
        if (graph.hasClassification(
                ShaderNode::Classification::SHADER |
                ShaderNode::Classification::SURFACE)) {
            // Emit all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitFunctionCalls(
                graph, context, stage, ShaderNode::Classification::TEXTURE);

            // Emit function calls for "root" closure/shader nodes.
            // These will internally emit function calls for any dependent
            // closure nodes upstream.
            for (ShaderGraphOutputSocket* socket : graph.getOutputSockets()) {
                if (socket->getConnection()) {
                    const ShaderNode* upstream =
                        socket->getConnection()->getNode();
                    if (upstream->getParent() == &graph &&
                        (upstream->hasClassification(
                             ShaderNode::Classification::CLOSURE) ||
                         upstream->hasClassification(
                             ShaderNode::Classification::SHADER))) {
                        emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else {
            // No surface shader graph so just generate all
            // function calls in order.
            emitFunctionCalls(graph, context, stage);
        }

        // Emit final output
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection) {
            string finalOutput = outputConnection->getVariable();
            const string& channels = outputSocket->getChannels();
            if (!channels.empty()) {
                finalOutput = _syntax->getSwizzledVariable(
                    finalOutput,
                    outputConnection->getType(),
                    channels,
                    outputSocket->getType());
            }

            if (graph.hasClassification(ShaderNode::Classification::SURFACE)) {
                if (context.getOptions().hwTransparency) {
                    emitLine(
                        "float outAlpha = clamp(1.0 - dot(" + finalOutput +
                            ".transparency, float3(0.3333)), 0.0, 1.0)",
                        stage);
                    emitLine(
                        outputSocket->getVariable() + " = float4(" +
                            finalOutput + ".color, outAlpha)",
                        stage);
                    emitLine(
                        "if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")",
                        stage,
                        false);
                    emitScopeBegin(stage);
                    emitLine("discard", stage);
                    emitScopeEnd(stage);
                }
                else {
                    emitLine(
                        outputSocket->getVariable() + " = float4(" +
                            finalOutput + ".color, 1.0)",
                        stage);
                }
            }
            else {
                if (!outputSocket->getType()->isFloat4()) {
                    toVec4(outputSocket->getType(), finalOutput);
                }
                emitLine(
                    outputSocket->getVariable() + " = " + finalOutput, stage);
            }
        }
        else {
            string outputValue =
                outputSocket->getValue()
                    ? _syntax->getValue(
                          outputSocket->getType(), *outputSocket->getValue())
                    : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType()->isFloat4()) {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(
                    _syntax->getTypeName(outputSocket->getType()) + " " +
                        finalOutput + " = " + outputValue,
                    stage);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine(
                    outputSocket->getVariable() + " = " + finalOutput, stage);
            }
            else {
                emitLine(
                    outputSocket->getVariable() + " = " + outputValue, stage);
            }
        }
    }

    // End main function
    emitFunctionBodyEnd(graph, context, stage);
}

void SlangShaderGenerator::emitLightFunctionDefinitions(
    const ShaderGraph& graph,
    GenContext& context,
    ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit Light functions if requested
        if (requiresLighting(graph) &&
            context.getOptions().hwMaxActiveLightSources > 0) {
            // For surface shaders we need light shaders
            if (graph.hasClassification(
                    ShaderNode::Classification::SHADER |
                    ShaderNode::Classification::SURFACE)) {
                // Emit functions for all bound light shaders
                HwLightShadersPtr lightShaders =
                    context.getUserData<HwLightShaders>(
                        HW::USER_DATA_LIGHT_SHADERS);
                if (lightShaders) {
                    for (const auto& it : lightShaders->get()) {
                        emitFunctionDefinition(*it.second, context, stage);
                    }
                }
                // Emit functions for light sampling
                for (const auto& it : _lightSamplingNodes) {
                    emitFunctionDefinition(*it, context, stage);
                }
            }
        }
    }
}

void SlangShaderGenerator::toVec4(const TypeDesc* type, string& variable)
{
    if (type->isFloat3()) {
        variable = "float4(" + variable + ", 1.0)";
    }
    else if (type->isFloat2()) {
        variable = "float4(" + variable + ", 0.0, 1.0)";
    }
    else if (*type == *Type::FLOAT || *type == *Type::INTEGER) {
        variable =
            "float4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (*type == *Type::BSDF || *type == *Type::EDF) {
        variable = "float4(" + variable + ", 1.0)";
    }
    else {
        // Can't understand other types. Just return black.
        variable = "float4(0.0, 0.0, 0.0, 1.0)";
    }
}

void SlangShaderGenerator::emitVariableDeclaration(
    const ShaderPort* variable,
    const string& qualifier,
    GenContext&,
    ShaderStage& stage,
    bool assignValue) const
{
    // A file texture input needs special handling on SLANG
    if (*variable->getType() == *Type::FILENAME) {
        // Samplers must always be uniforms
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "Sampler2D " + variable->getVariable(), stage);
    }
    else {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        // Varying parameters of type int must be flat qualified on output from
        // vertex stage and input to pixel stage. The only way to get these is
        // with geompropvalue_integer nodes.
        if (qualifier.empty() && *variable->getType() == *Type::INTEGER &&
            !assignValue &&
            variable->getName().rfind(HW::T_IN_GEOMPROP, 0) == 0) {
            str += SlangSyntax::FLAT_QUALIFIER + " ";
        }
        str += _syntax->getTypeName(variable->getType()) + " " +
               variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType()->isArray() && variable->getValue()) {
            str += _syntax->getArrayVariableSuffix(
                variable->getType(), *variable->getValue());
        }

        if (!variable->getSemantic().empty()) {
            str += " : " + variable->getSemantic();
        }

        if (assignValue) {
            const string valueStr =
                (variable->getValue()
                     ? _syntax->getValue(
                           variable->getType(), *variable->getValue(), true)
                     : _syntax->getDefaultValue(variable->getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

ShaderNodeImplPtr SlangShaderGenerator::getImplementation(
    const NodeDef& nodedef,
    GenContext& context) const
{
    InterfaceElementPtr implElement = nodedef.getImplementation(getTarget());
    if (!implElement) {
        return nullptr;
    }

    const string& name = implElement->getName();

    // Check if it's created and cached already.
    ShaderNodeImplPtr impl = context.findNodeImplementation(name);
    if (impl) {
        return impl;
    }

    vector<OutputPtr> outputs = nodedef.getActiveOutputs();
    if (outputs.empty()) {
        throw ExceptionShaderGenError(
            "NodeDef '" + nodedef.getName() + "' has no outputs defined");
    }

    const TypeDesc* outputType = TypeDesc::get(outputs[0]->getType());

    if (implElement->isA<NodeGraph>()) {
        // Use a compound implementation.
        if (*outputType == *Type::LIGHTSHADER) {
            impl = LightCompoundNodeSlang::create();
        }
        else if (outputType->isClosure()) {
            impl = ClosureCompoundNode::create();
        }
        else {
            impl = CompoundNodeSlang::create();
        }
    }
    else if (implElement->isA<Implementation>()) {
        // Try creating a new in the factory.
        impl = _implFactory.create(name);
        if (!impl) {
            // Fall back to source code implementation.
            if (outputType->isClosure()) {
                impl = ClosureSourceCodeNode::create();
            }
            else {
                impl = SourceCodeNode::create();
            }
        }
    }
    if (!impl) {
        return nullptr;
    }

    impl->initialize(*implElement, context);

    // Cache it.
    context.addNodeImplementation(name, impl);

    return impl;
}

void SlangShaderGenerator::emitLibraryInclude(
    const FilePath& filename,
    GenContext& context,
    ShaderStage& stage) const
{
    auto name = filename;
    name.removeExtension();
    auto file_path = name.asString(FilePath::FormatPosix);

    std::ranges::replace(file_path, '/', '.');

    auto line = std::format("import {0}", file_path);
    emitLine(line, stage);
}

void SlangShaderGenerator::emitBlock(
    const string& str,
    const FilePath& sourceFilename,
    GenContext& context,
    ShaderStage& stage) const
{
    const string& INCLUDE = _syntax->getIncludeStatement();
    const string& QUOTE = _syntax->getStringQuote();

    // Add each line in the block seperately to get correct indentation.
    StringStream stream(str);
    for (string line; std::getline(stream, line);) {
        size_t pos = line.find(INCLUDE);
        if (pos != string::npos) {
            size_t startQuote = line.find_first_of(QUOTE);
            size_t endQuote = line.find_last_of(QUOTE);
            if (startQuote != string::npos && endQuote != string::npos &&
                endQuote > startQuote) {
                FilePath sourceFilePath = sourceFilename.getParentPath();
                FilePath relativePath;

                while (sourceFilePath.getBaseName() != "libraries") {
                    relativePath =
                        FilePath(sourceFilePath[sourceFilePath.size() - 1]) /
                        relativePath;

                    sourceFilePath = sourceFilePath.getParentPath();
                }

                size_t length = (endQuote - startQuote) - 1;
                if (length) {
                    const string filename = line.substr(startQuote + 1, length);
                    emitLibraryInclude(
                        relativePath.asString() + filename, context, stage);
                }
            }
        }
        else {
            emitLine(line, stage, false);
        }
    }
}

const string& SlangImplementation::getTarget() const
{
    return SlangShaderGenerator::TARGET;
}

MATERIALX_NAMESPACE_END
