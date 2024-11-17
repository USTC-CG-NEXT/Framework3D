#include <gtest/gtest.h>

#include "../source/material/materialx.hpp"

using namespace USTC_CG;
TEST(MATERIALX, shader_gen)
{
    MaterialX::ShaderGeneratorPtr ptr =
        std::make_shared<MaterialXGenSlang>(nullptr);
}