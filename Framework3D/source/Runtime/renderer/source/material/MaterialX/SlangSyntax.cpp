//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "SlangSyntax.h"

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace {

// Since SLANG doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class SlangStringTypeSyntax : public StringTypeSyntax {
   public:
    SlangStringTypeSyntax() : StringTypeSyntax("int", "0", "0")
    {
    }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class SlangArrayTypeSyntax : public ScalarTypeSyntax {
   public:
    SlangArrayTypeSyntax(const string& name)
        : ScalarTypeSyntax(name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        size_t arraySize = getSize(value);
        if (arraySize > 0) {
            return _name + "[" + std::to_string(arraySize) + "](" +
                   value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

    string getValue(const StringVec& values, bool /*uniform*/) const override
    {
        if (values.empty()) {
            throw ExceptionShaderGenError(
                "No values given to construct an array value");
        }

        string result =
            _name + "[" + std::to_string(values.size()) + "](" + values[0];
        for (size_t i = 1; i < values.size(); ++i) {
            result += ", " + values[i];
        }
        result += ")";

        return result;
    }

   protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class SlangFloatArrayTypeSyntax : public SlangArrayTypeSyntax {
   public:
    explicit SlangFloatArrayTypeSyntax(const string& name)
        : SlangArrayTypeSyntax(name)
    {
    }

   protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class SlangIntegerArrayTypeSyntax : public SlangArrayTypeSyntax {
   public:
    explicit SlangIntegerArrayTypeSyntax(const string& name)
        : SlangArrayTypeSyntax(name)
    {
    }

   protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

}  // anonymous namespace

const string SlangSyntax::INPUT_QUALIFIER = "in";
const string SlangSyntax::OUTPUT_QUALIFIER = "out";
const string SlangSyntax::UNIFORM_QUALIFIER = "cbuffer";
const string SlangSyntax::CONSTANT_QUALIFIER = "const";
const string SlangSyntax::FLAT_QUALIFIER = "flat";
const string SlangSyntax::SOURCE_FILE_EXTENSION = ".slang";
const StringVec SlangSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec SlangSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec SlangSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// SlangSyntax methods
//

SlangSyntax::SlangSyntax()
{
    // Add in all reserved words and keywords in SLANG
    registerReservedWords({ "centroid",
                            "flat",
                            "smooth",
                            "noperspective",
                            "patch",
                            "sample",
                            "break",
                            "continue",
                            "do",
                            "for",
                            "while",
                            "switch",
                            "case",
                            "default",
                            "if",
                            "else,",
                            "subroutine",
                            "in",
                            "out",
                            "inout",
                            "float",
                            "double",
                            "int",
                            "void",
                            "bool",
                            "true",
                            "false",
                            "invariant",
                            "discard",
                            "return",
                            "mat2",
                            "float3x3",
                            "float4x4",
                            "dmat2",
                            "dmat3",
                            "dmat4",
                            "mat2x2",
                            "mat2x3",
                            "mat2x4",
                            "dmat2x2",
                            "dmat2x3",
                            "dmat2x4",
                            "mat3x2",
                            "mat3x3",
                            "mat3x4",
                            "dmat3x2",
                            "dmat3x3",
                            "dmat3x4",
                            "mat4x2",
                            "mat4x3",
                            "mat4x4",
                            "dmat4x2",
                            "dmat4x3",
                            "dmat4x4",
                            "float2",
                            "float3",
                            "float4",
                            "ifloat2",
                            "ifloat3",
                            "ifloat4",
                            "bfloat2",
                            "bfloat3",
                            "bfloat4",
                            "dfloat2",
                            "dfloat3",
                            "dfloat4",
                            "uint",
                            "ufloat2",
                            "ufloat3",
                            "ufloat4",
                            "lowp",
                            "mediump",
                            "highp",
                            "precision",
                            "sampler1D",
                            "Sampler2D",
                            "sampler3D",
                            "samplerCube",
                            "sampler1DShadow",
                            "sampler2DShadow",
                            "samplerCubeShadow",
                            "sampler1DArray",
                            "sampler2DArray",
                            "sampler1DArrayShadow",
                            "sampler2DArrayShadow",
                            "isampler1D",
                            "isampler2D",
                            "isampler3D",
                            "isamplerCube",
                            "isampler1DArray",
                            "isampler2DArray",
                            "usampler1D",
                            "usampler2D",
                            "usampler3D",
                            "usamplerCube",
                            "usampler1DArray",
                            "usampler2DArray",
                            "sampler2DRect",
                            "sampler2DRectShadow",
                            "isampler2DRect",
                            "usampler2DRect",
                            "samplerBuffer",
                            "isamplerBuffer",
                            "usamplerBuffer",
                            "sampler2DMS",
                            "isampler2DMS",
                            "usampler2DMS",
                            "sampler2DMSArray",
                            "isampler2DMSArray",
                            "usampler2DMSArray",
                            "samplerCubeArray",
                            "samplerCubeArrayShadow",
                            "isamplerCubeArray",
                            "usamplerCubeArray",
                            "common",
                            "partition",
                            "active",
                            "asm",
                            "struct",
                            "class",
                            "union",
                            "enum",
                            "typedef",
                            "template",
                            "this",
                            "packed",
                            "goto",
                            "inline",
                            "noinline",
                            "volatile",
                            "public",
                            "static",
                            "extern",
                            "external",
                            "interface",
                            "long",
                            "short",
                            "half",
                            "fixed",
                            "unsigned",
                            "superp",
                            "input",
                            "output",
                            "hfloat2",
                            "hfloat3",
                            "hfloat4",
                            "ffloat2",
                            "ffloat3",
                            "ffloat4",
                            "sampler3DRect",
                            "filter",
                            "image1D",
                            "image2D",
                            "image3D",
                            "imageCube",
                            "iimage1D",
                            "iimage2D",
                            "iimage3D",
                            "iimageCube",
                            "uimage1D",
                            "uimage2D",
                            "uimage3D",
                            "uimageCube",
                            "image1DArray",
                            "image2DArray",
                            "iimage1DArray",
                            "iimage2DArray",
                            "uimage1DArray",
                            "uimage2DArray",
                            "image1DShadow",
                            "image2DShadow",
                            "image1DArrayShadow",
                            "image2DArrayShadow",
                            "imageBuffer",
                            "iimageBuffer",
                            "uimageBuffer",
                            "sizeof",
                            "cast",
                            "namespace",
                            "using",
                            "row_major",
                            "lerp",
                            "sampler" });

    // Register restricted tokens in SLANG
    StringMap tokens;
    tokens["__"] = "_";
    tokens["gl_"] = "gll";
    tokens["webgl_"] = "webgll";
    tokens["_webgl"] = "wwebgl";
    registerInvalidTokens(tokens);

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT, std::make_shared<ScalarTypeSyntax>("float", "0.0", "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY, std::make_shared<SlangFloatArrayTypeSyntax>("float"));

    registerTypeSyntax(
        Type::INTEGER, std::make_shared<ScalarTypeSyntax>("int", "0", "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<SlangIntegerArrayTypeSyntax>("int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>("bool", "false", "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            "float2",
            "float2(0.0)",
            "float2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            "float3x3", "float3x3(1.0)", "float3x3(1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            "float4x4", "float4x4(1.0)", "float4x4(1.0)"));

    registerTypeSyntax(Type::STRING, std::make_shared<SlangStringTypeSyntax>());

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            "Sampler2D", EMPTY_STRING, EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            "BSDF",
            "BSDF(float3(0.0),float3(1.0), 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; float "
            "thickness; float ior; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            "EDF", "EDF(0.0)", "EDF(0.0)", "float3", "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            "BSDF", "BSDF(float3(0.0),float3(1.0), 0.0, 0.0)", EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "surfaceshader",
            "surfaceshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "volumeshader",
            "volumeshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "displacementshader",
            "displacementshader(float3(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "lightshader",
            "lightshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<AggregateTypeSyntax>(
            "material",
            "material(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool SlangSyntax::typeSupported(const TypeDesc* type) const
{
    return type != Type::STRING;
}

bool SlangSyntax::remapEnumeration(
    const string& value,
    const TypeDesc* type,
    const string& enumNames,
    std::pair<const TypeDesc*, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty()) {
        return false;
    }

    // Don't convert already supported types
    // or filenames and arrays.
    if (typeSupported(type) || *type == *Type::FILENAME ||
        (type && type->isArray())) {
        return false;
    }

    // For SLANG we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    // Try remapping to an enum value.
    if (!value.empty()) {
        StringVec valueElemEnumsVec = splitString(enumNames, ",");
        for (size_t i = 0; i < valueElemEnumsVec.size(); i++) {
            valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
        }
        auto pos = std::find(
            valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
        if (pos == valueElemEnumsVec.end()) {
            throw ExceptionShaderGenError(
                "Given value '" + value +
                "' is not a valid enum value for input.");
        }
        const int index =
            static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
        result.second = Value::createValue<int>(index);
    }

    return true;
}

MATERIALX_NAMESPACE_END
