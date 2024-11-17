#include <gtest/gtest.h>

#include "../source/material/MaterialX/GlslShaderGenerator.h"

TEST(MATERIALX, shader_gen)
{
    MaterialX::ShaderGeneratorPtr ptr =
        std::make_shared<MaterialX::GlslShaderGenerator>();
}