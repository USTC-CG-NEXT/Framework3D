#include <gtest/gtest.h>

#include <glintify/mesh.hpp>

TEST(Mesh, load_from_obj)
{
    auto mesh = USTC_CG::Mesh::load_from_obj("cube.obj");
    ASSERT_EQ(mesh.vertices.size(), 8);

    ASSERT_EQ(mesh.indices.size(), 24);

    auto edge_samples = mesh.sample_on_edges(0.099f);

    ASSERT_EQ(edge_samples.size(), 12 * 11);
}