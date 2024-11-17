#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#include <MaterialXGenShader/Util.h>
#include <gtest/gtest.h>

#include <fstream>

#include "../source/material/MaterialX/GlslResourceBindingContext.h"
#include "../source/material/MaterialX/GlslShaderGenerator.h"
#include "../source/material/MaterialX/GlslSyntax.h"
#include "../source/material/MaterialX/VkResourceBindingContext.h"
#include "../source/material/MaterialX/VkShaderGenerator.h"
#include "glsl_tester.hpp"

namespace mx = MaterialX;

void checkPixelDependencies(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath testFile = searchPath.find(
        "resources/Materials/Examples/GltfPbr/gltf_pbr_boombox.mtlx");
    mx::string testElement = "Material_boombox";

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlFile(testDoc, testFile);
    testDoc->importLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    ASSERT_TRUE(element);

    mx::ShaderPtr shader =
        context.getShaderGenerator().generate(testElement, element, context);
    std::set<std::string> dependencies =
        shader->getStage("pixel").getSourceDependencies();
    for (auto dependency : dependencies) {
        mx::FilePath path(dependency);
        std::cout << dependency << std::endl;
        ASSERT_TRUE(path.exists() == true);
    }
}

TEST(MATERIALX, shader_gen)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    auto str = prettyPrint(libraries);

    std::ofstream outFile("mtlx_libraries.txt");
    outFile << str;
    outFile.close();
    using namespace mx;
    mx::GenContext context(mx::GlslShaderGenerator::create());

    context.registerSourceCodeSearchPath(searchPath);
    checkPixelDependencies(libraries, context);
}

TEST(GenShader, Syntax_Check)
{
    mx::SyntaxPtr syntax = mx::GlslSyntax::create();

    ASSERT_TRUE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    ASSERT_TRUE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
    ASSERT_TRUE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

    ASSERT_TRUE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    ASSERT_TRUE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

    // Set fixed precision with one digit
    mx::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    ASSERT_TRUE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    ASSERT_TRUE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    ASSERT_TRUE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    ASSERT_TRUE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    ASSERT_TRUE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::FLOATARRAY, true);
    ASSERT_TRUE(value.empty());
    value = syntax->getDefaultValue(mx::Type::INTEGERARRAY, true);
    ASSERT_TRUE(value.empty());

    mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
    value = syntax->getValue(mx::Type::FLOAT, *floatValue);
    ASSERT_TRUE(value == "42.0");
    value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
    ASSERT_TRUE(value == "42.0");

    mx::ValuePtr color3Value =
        mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
    value = syntax->getValue(mx::Type::COLOR3, *color3Value);
    ASSERT_TRUE(value == "vec3(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    ASSERT_TRUE(value == "vec3(1.0, 2.0, 3.0)");

    mx::ValuePtr color4Value =
        mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    ASSERT_TRUE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    ASSERT_TRUE(value == "vec4(1.0, 2.0, 3.0, 4.0)");

    std::vector<float> floatArray = {
        0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f
    };
    mx::ValuePtr floatArrayValue =
        mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    ASSERT_TRUE(value == "float[7](0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue =
        mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    ASSERT_TRUE(value == "int[7](1, 2, 3, 4, 5, 6, 7)");
}

TEST(GenShader, GLSL_Implementation)
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    mx::StringSet generatorSkipNodeDefs;
    USTC_CG::GenShaderUtil::checkImplementations(
        context, generatorSkipNodeTypes, generatorSkipNodeDefs, 47);
}

TEST(GenShader, GLSL_Unique_Names)
{
    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(mx::getDefaultDataSearchPath());
    USTC_CG::GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

TEST(GenShader, Bind_Light_Shaders)
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    loadLibraries({ "libraries" }, searchPath, doc);

    mx::NodeDefPtr pointLightShader = doc->getNodeDef("ND_point_light");
    mx::NodeDefPtr spotLightShader = doc->getNodeDef("ND_spot_light");
    ASSERT_TRUE(pointLightShader != nullptr);
    ASSERT_TRUE(spotLightShader != nullptr);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);

    mx::HwShaderGenerator::bindLightShader(*pointLightShader, 42, context);
    ASSERT_ANY_THROW(
        mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    mx::HwShaderGenerator::unbindLightShader(42, context);
    ASSERT_NO_THROW(
        mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    ASSERT_NO_THROW(
        mx::HwShaderGenerator::bindLightShader(*pointLightShader, 66, context));
    mx::HwShaderGenerator::unbindLightShaders(context);
    ASSERT_NO_THROW(
        mx::HwShaderGenerator::bindLightShader(*spotLightShader, 66, context));
}

enum class GlslType { Glsl400, Glsl420, GlslVulkan };

const std::string GlslTypeToString(GlslType e) throw()
{
    switch (e) {
        case GlslType::Glsl420: return "glsl420_layout";
        case GlslType::GlslVulkan: return "glsl420_vulkan";
        case GlslType::Glsl400:
        default: return "glsl400";
    }
}

static void generateGlslCode(GlslType type = GlslType::Glsl400)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(searchPath.find("resources/Materials/TestSuite"));
    testRootPaths.push_back(
        searchPath.find("resources/Materials/Examples/StandardSurface"));

    const mx::FilePath logPath(
        "genglsl_" + GlslTypeToString(type) + "_generate_test.txt");

    bool writeShadersToDisk = false;
    USTC_CG::GenShaderUtil::GlslShaderGeneratorTester tester(
        (type == GlslType::GlslVulkan) ? mx::VkShaderGenerator::create()
                                       : mx::GlslShaderGenerator::create(),
        testRootPaths,
        searchPath,
        logPath,
        writeShadersToDisk);

    // Add resource binding context for glsl 4.20
    if (type == GlslType::Glsl420) {
        // Set binding context to handle resource binding layouts
        mx::GlslResourceBindingContextPtr glslresourceBinding(
            mx::GlslResourceBindingContext::create());
        glslresourceBinding->enableSeparateBindingLocations(true);
        tester.addUserData(
            mx::HW::USER_DATA_BINDING_CONTEXT, glslresourceBinding);
    }

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath =
        searchPath.find("resources/Materials/TestSuite/_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST(GenShader, GLSL_ShaderGeneration)
{
    // Generate with standard GLSL i.e version 400
    generateGlslCode(GlslType::Glsl400);
}

TEST(GenShader, GLSL_Shader_with_Layout_Generation)
{
    // Generate GLSL with layout i.e version 400 + layout extension
    generateGlslCode(GlslType::Glsl420);
}

TEST(GenShader, Vulkan_GLSL_Shader)
{
    // Generate with GLSL for Vulkan i.e. version 450
    generateGlslCode(GlslType::GlslVulkan);
}
