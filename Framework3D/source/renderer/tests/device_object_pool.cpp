#include <gtest/gtest.h>

#include "../source/internal/memory/DeviceObjectPool.hpp"
// SceneTypes
#include "../nodes/shaders/shaders/Scene/SceneTypes.slang"
using namespace USTC_CG;
TEST(MemoryPool, create)
{
    DeviceObjectPool<GeometryInstanceData> pool;
}

TEST(MemoryPool, insert)
{
    DeviceObjectPool<GeometryInstanceData> pool;
    GeometryInstanceData data;
    auto handle = pool.insert(data);
    ASSERT_TRUE(handle.isValid());
}

TEST(MemoryPool, erase)
{
    DeviceObjectPool<GeometryInstanceData> pool;
    GeometryInstanceData data;
    auto handle = pool.insert(data);
    pool.erase(handle);
    ASSERT_FALSE(handle.isValid());
}

