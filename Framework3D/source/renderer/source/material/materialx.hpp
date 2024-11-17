#pragma once
#include "../api.h"
#include "MaterialXGenShader/Shader.h"
#include "MaterialXGenShader/ShaderGenerator.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class MaterialXGenSlang final : public MaterialX::ShaderGenerator {
public:
    explicit MaterialXGenSlang(const MaterialX_v1_38_10::SyntaxPtr& syntax)
        : ShaderGenerator(syntax)
    {
    }

};

USTC_CG_NAMESPACE_CLOSE_SCOPE